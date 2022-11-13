#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "neopixel.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/structs/bus_ctrl.h"


#define QTPY_BOOT_PIN 21
#define LED_CTRL_PIN 29

const uint CAPTURE_PIN_BASE = 22;
const uint CAPTURE_PIN_COUNT = 2;
const uint CAPTURE_N_SAMPLES = 9600;
const uint TRIGGER_PIN = 21;

/************************************PIO Helpers Definition Starts*****************************************/
static inline uint bits_packed_per_word(uint pin_count) {
    // If the number of pins to be sampled divides the shift register size, we
    // can use the full SR and FIFO width, and push when the input shift count
    // exactly reaches 32. If not, we have to push earlier, so we use the FIFO
    // a little less efficiently.
    const uint SHIFT_REG_WIDTH = 32;
    return SHIFT_REG_WIDTH - (SHIFT_REG_WIDTH % pin_count);
}

void logic_analyser_init(PIO pio, uint sm, uint pin_base, uint pin_count, float div) {
    // Load a program to capture n pins. This is just a single `in pins, n`
    // instruction with a wrap.
    uint16_t capture_prog_instr = pio_encode_in(pio_pins, pin_count);
    struct pio_program capture_prog = {
            .instructions = &capture_prog_instr,
            .length = 1,
            .origin = -1
    };
    uint offset = pio_add_program(pio, &capture_prog);

    // Configure state machine to loop over this `in` instruction forever,
    // with autopush enabled.
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, pin_base);
    sm_config_set_wrap(&c, offset, offset);
    sm_config_set_clkdiv(&c, div);
    // Note that we may push at a < 32 bit threshold if pin_count does not
    // divide 32. We are using shift-to-right, so the sample data ends up
    // left-justified in the FIFO in this case, with some zeroes at the LSBs.
    sm_config_set_in_shift(&c, true, true, bits_packed_per_word(pin_count));
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio, sm, offset, &c);
}

void logic_analyser_arm(PIO pio, uint sm, uint dma_chan, uint32_t *capture_buf, size_t capture_size_words,
                        uint trigger_pin, bool trigger_level) {
    pio_sm_set_enabled(pio, sm, false);
    // Need to clear _input shift counter_, as well as FIFO, because there may be
    // partial ISR contents left over from a previous run. sm_restart does this.
    pio_sm_clear_fifos(pio, sm);
    pio_sm_restart(pio, sm);

    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));

    dma_channel_configure(dma_chan, &c,
        capture_buf,        // Destination pointer
        &pio->rxf[sm],      // Source pointer
        capture_size_words, // Number of transfers
        true                // Start immediately
    );

    // wait until the trigger_level detected
    pio_sm_set_enabled(pio, sm, true);
}

void print_capture_buf(const uint32_t *buf, uint pin_base, uint pin_count, uint32_t n_samples, int *timeStmp) {
    // Display the capture buffer in text form, like this:
    // 00: __--__--__--__--__--__--
    // 01: ____----____----____----
    printf("Capture:\n");
    // Each FIFO record may be only partially filled with bits, depending on
    // whether pin_count is a factor of 32.
    uint record_size_bits = bits_packed_per_word(pin_count);
    uint captured_bits[n_samples];
    for (int pin = 0; pin < pin_count; ++pin) {
        for (int sample = 1; sample < n_samples; ++sample) {
            uint bit_index = pin + sample * pin_count;
            uint word_index = bit_index / record_size_bits;
            // Data is left-justified in each FIFO entry, hence the (32 - record_size_bits) offset
            uint word_mask = 1u << (bit_index % record_size_bits + 32 - record_size_bits);
            uint current_Data_Bit = buf[word_index] & word_mask;

            // TODO logic of extracting transition 
            if(buf[word_index] & word_mask){
                captured_bits[sample] = 1;
            }else{
                captured_bits[sample] = 0;
            }

            if(captured_bits[sample] != captured_bits[sample - 1]){
               printf("pin num: %02d| data bit: %d| time:%d \n",(pin+pin_base), captured_bits[sample], timeStmp[sample]); 
            }
            
        }
    }
}

// sio base is 0xd0000000
// gpio input offset is 0x00000004
// full register address is 0xd0000004

/************************************REPL Helpers Definition Starts*****************************************/
typedef struct
{
    long timeStamp;
    uint32_t target_address;
    uint32_t address_content;
    uint32_t write_content;
    uint32_t mask_value;
    float factor;
    bool isRead;
    bool isAddress;
    bool isMask;
    char addressChars[8];
    char contentChars[8];
} ReplStatus;

void render_to_console(ReplStatus status, char inputChars[], ReplStatus prevStatus)
{
    if (prevStatus.address_content != status.address_content)
    {
        // adjust console window height to match 'frame size'
        for (uint8_t i = 0; i < 10; i++)
        {
            printf("\n");
        }
        printf("Current Register:  0x%08x\n", status.target_address);
        printf("Value:        0x%08x\n", status.address_content);
        printf("Read Mode?: %d\n", status.isRead);
        printf("input: %s \n", inputChars);
        printf("write_content: 0x%08x \n", status.write_content);
        printf("Mask Value: 0x%08x \n", status.mask_value);
        printf("Current Input is Address: %d \n", status.isAddress);
        printf("Current Input is Mask: %d \n", status.isMask);
        printf("REPL Running Speed Factor (base=10Hz): %01f \n", status.factor);
        printf("Current timeStamp is: %d \n", status.timeStamp);
    }
}

uint32_t get_register_content(uint32_t address)
{
    volatile uint32_t *register_address;
    volatile uint32_t full_register_value;

    register_address = (volatile uint32_t *)address;
    full_register_value = (uint32_t)*register_address;
    return full_register_value;
}

void set_register_content(uint32_t address, uint32_t value, uint32_t mask)
{
    volatile uint32_t *register_address;
    volatile uint32_t current_reg_value;
    volatile uint32_t value_after_masking;
    uint32_t currentBit;
    uint32_t currentMaskBit;
    uint32_t currentDataBit;
    uint32_t finalDataBit;
    uint32_t finalWholeData = 0;

    register_address = (volatile uint32_t *)address; // pointer
    current_reg_value = (uint32_t)*register_address; // value
    value_after_masking = (*register_address ^ value) & mask;

    if (current_reg_value != value)
    {
        if (address >= 0xd0000000 && address <= 0xd000017c)
        { // register is SIO register, only 32-bit whole register write allowed
            for (int i = 0; i < 32; i++)
            {
                currentBit = current_reg_value % 2;
                currentDataBit = value % 2;
                currentMaskBit = mask % 2;

                if (currentMaskBit == 1)
                {
                    finalDataBit = currentDataBit;
                }
                else
                {
                    finalDataBit = currentBit;
                }

                finalWholeData = finalWholeData + pow(2, i) * finalDataBit;

                mask = mask / 2;
                current_reg_value = current_reg_value / 2;
                value = value / 2;
            }
            *register_address = finalWholeData;
        }
        else
        {
            // hw_xor_bits(register_address, value_after_masking);
            //*(io_rw_32 *) hw_xor_alias_untyped((volatile void *) register_address) = value_after_masking;
            *(io_rw_32 *)((void *)(REG_ALIAS_XOR_BITS | (uintptr_t)(register_address))) = value_after_masking;
        }
    }
}

uint32_t get_uint32_from_string(char charArray[])
{
    int p = 0;
    int power = 7;
    uint32_t result = 0x00000000;
    uint32_t currentBitNumber = 0x0;
    int currentnumber = 0;
    while (p < 8)
    {
        if (charArray[p])
        {
            if (charArray[p] >= '0' && charArray[p] <= '9')
            {
                currentnumber = (charArray[p] - '0');
            }
            else if (charArray[p] >= 'A' && charArray[p] <= 'F')
            {
                currentnumber = (charArray[p] - 'A') + 10;
            }
            else
            {
                currentnumber = (charArray[p] - 'a') + 10;
            }

            currentBitNumber = (uint32_t)currentnumber * (pow(0x10, power));
            result = result + currentBitNumber;
            p++;
            power--;
        }
    }

    return result;
}

int main()
{
    /************************************PIO Setup Code Starts*****************************************/
    stdio_init_all();
    gpio_init(QTPY_BOOT_PIN);
    gpio_set_dir(QTPY_BOOT_PIN, 0);
    neopixel_init();
    gpio_init(CAPTURE_PIN_BASE);
    gpio_set_dir(CAPTURE_PIN_BASE, 0);
    gpio_init(CAPTURE_PIN_BASE + 1);
    gpio_set_dir(CAPTURE_PIN_BASE + 1, 0);

    // memory allocation of the capture_buf
    uint total_sample_bits = CAPTURE_N_SAMPLES * CAPTURE_PIN_COUNT;
    total_sample_bits += bits_packed_per_word(CAPTURE_PIN_COUNT) - 1;
    uint buf_size_words = total_sample_bits / bits_packed_per_word(CAPTURE_PIN_COUNT);
    uint32_t *capture_buf = malloc(buf_size_words * sizeof(uint32_t));
    hard_assert(capture_buf);

    uint timeStamp[CAPTURE_N_SAMPLES];

    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;

    PIO pio = pio0;
    uint sm = 0;
    uint dma_chan = 0;

    printf("10s");
    //sleep_ms(10*1000);
    printf("Logic Init \n");
    //sleep_ms(5*1000);
    logic_analyser_init(pio, sm, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, 1.f);

    printf("Arming trigger\n");

    uint pioCounter = 0;
    uint record_size_bits = bits_packed_per_word(CAPTURE_PIN_COUNT);
    uint captured_bits[CAPTURE_N_SAMPLES];
    uint lastBitPin1 = 0;
    uint lastBitPin2 = 0;

    /************************************REPL Setup Code Starts*****************************************/

    volatile char inputchars[9];
    volatile char readOrWrite;
    volatile char inputch;
    volatile uint32_t register_value;
    volatile uint32_t register_address;
    int counter = 0;

    volatile uint32_t *pin_29_address;
    volatile uint32_t full_gpio_register_value;
    volatile uint32_t pin_29_selection_mask = 1u << 29;
    volatile uint32_t selected_pin_state;
    volatile uint32_t shifted_pin_29_state;

    ReplStatus status;
    ReplStatus prevStatus;

    status.timeStamp = 0;
    status.target_address = 0x00000000;
    status.address_content = 0x00000000;
    status.isRead = true;
    status.isAddress = true;
    status.isMask = false;
    status.factor = 1.0;

    prevStatus = status;

    inputchars[8] = '\0';

    while (true)
    {
        /************** REPL Code Starts **************/
        // keep grabbing data from the console if the array is not full
        if (counter < 8)
        {
            inputch = getchar_timeout_us(0); // don't block main loop
            // mode selection
            if (inputch == 'R' || inputch == 'r')
            {
                status.isRead = true;
            }
            else if (inputch == 'W' || inputch == 'w')
            {
                status.isRead = false;
            }else if(inputch == 'X'){
                status.factor = status.factor * 2;
            }else if(inputch == 'x'){
                status.factor = status.factor / 2.0;
            }
            else
            {
                // if the input is not the mode control and it is a valid hex number, then save it in the array
                if ((inputch <= '9' && inputch >= '0') || (inputch >= 'A' && inputch <= 'F') || (inputch >= 'a' && inputch <= 'f'))
                {
                    inputchars[counter] = inputch;
                    counter++;
                }
            }
        }
        else
        {
            if (status.isAddress)
            {
                // the array is full, get the uin32 address from the char array.
                status.target_address = get_uint32_from_string(inputchars);
                if (status.target_address % 4 != 0)
                {
                    status.target_address = 0;
                }
            }
            else if (status.isMask)
            {
                status.mask_value = get_uint32_from_string(inputchars);
            }
            else
            {
                status.write_content = get_uint32_from_string(inputchars);
            }

            // reset the array
            while (counter > 0)
            {
                inputchars[counter - 1] = ' ';
                counter--;
            }

            // process the input content and save it to the correct place
            // if last time it is address, then next step is the mask
            if (status.isAddress)
            {
                status.isAddress = false;
                status.isMask = true;
            }
            else if (status.isMask && status.isRead)
            { // if last time it is mask and mode is reading, next step s address
                status.isAddress = true;
                status.isMask = false;
            }
            else if (status.isMask && (!status.isRead))
            { // if last time it is mask and mode is writing, next step is content
                status.isAddress = false;
                status.isMask = false;
            }
            else if ((!status.isAddress) && (!status.isMask))
            { // if last time it is content, next step is address
                status.isAddress = true;
                status.isMask = false;
            }
            else
            {
                status.isAddress = true;
                status.isMask = false;
            }

            // finally, counter is 0, inputchars will be cleared. The data is saved in the status
        }

        // apply mask
        if (status.isRead)
        {
            status.address_content = get_register_content(status.target_address);
            if (status.mask_value == 0)
            {
                status.address_content = 0;
            }
            else if (status.mask_value)
            {
                status.address_content = status.address_content & status.mask_value;
            }
        }
        else
        {
            if (status.isAddress == true)
            {
                set_register_content(status.target_address, status.write_content, status.mask_value);
            }
            status.address_content = get_register_content(status.target_address);
        }

        render_to_console(status, inputchars, prevStatus);
        prevStatus = status;
        sleep_ms(1/1000000/status.factor); // don't DDOS the serial console
        status.timeStamp = status.timeStamp + 1;

        pin_29_address = (volatile uint32_t *)0xd0000010;     // the pointer of GPIO_SET register
        full_gpio_register_value = (uint32_t)*pin_29_address; // retrive the register value lies in the address
        pin_29_selection_mask = 1u << 29;                     // mask used to retrieve the 29th bit
        selected_pin_state = full_gpio_register_value & pin_29_selection_mask;
        shifted_pin_29_state = selected_pin_state >> 29; // retrive the bit value


        /***********************************************PIO Processing Code Starts*********************************************************/
        uint sampNum = 0;

        if(!gpio_get(TRIGGER_PIN)){
            logic_analyser_arm(pio, sm, dma_chan, capture_buf, buf_size_words, CAPTURE_PIN_BASE, false);
            for(sampNum=0; sampNum<CAPTURE_N_SAMPLES; sampNum++){
                timeStamp[sampNum] = sampNum + pioCounter * CAPTURE_N_SAMPLES;
            }


            //print_capture_buf(capture_buf, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, CAPTURE_N_SAMPLES, timeStamp);
            // Each FIFO record may be only partially filled with bits, depending on
            // whether pin_count is a factor of 32.
            
            for (int pin = 0; pin < CAPTURE_PIN_COUNT; ++pin) {
                for (int sample = 0; sample < CAPTURE_N_SAMPLES; ++sample) {
                    uint bit_index = pin + sample * CAPTURE_PIN_COUNT;
                    uint word_index = bit_index / record_size_bits;
                    // Data is left-justified in each FIFO entry, hence the (32 - record_size_bits) offset
                    uint word_mask = 1u << (bit_index % record_size_bits + 32 - record_size_bits);
                    uint current_Data_Bit = capture_buf[word_index] & word_mask;

                    // transfer the data to bit
                    if(capture_buf[word_index] & word_mask){
                        captured_bits[sample] = 1;
                    }else{
                        captured_bits[sample] = 0;
                    }

                    if(sample == 0){
                        if(pin == 0){
                            if(captured_bits[sample] != lastBitPin1){
                                printf("pin num: %02d| data bit: %d| time:%d \n",(pin+CAPTURE_PIN_BASE), captured_bits[sample], timeStamp[sample]);
                            }
                        }else{
                            if(captured_bits[sample] != lastBitPin2){
                                printf("pin num: %02d| data bit: %d| time:%d \n",(pin+CAPTURE_PIN_BASE), captured_bits[sample], timeStamp[sample]);
                            }
                        }
                        
                    }else{
                        if(captured_bits[sample] != captured_bits[sample - 1]){
                            printf("pin num: %02d| data bit: %d| time:%d \n",(pin+CAPTURE_PIN_BASE), captured_bits[sample], timeStamp[sample]); 
                        }
                    }

                    // save the last bit of the last capture for the first bit comparision of the next capture
                    if(sample == CAPTURE_N_SAMPLES-1){
                        if(pin == 0){
                            lastBitPin1 = captured_bits[sample];
                        }else{
                            lastBitPin2 = captured_bits[sample];
                        }
                    }                                       
                }
            }


            pioCounter++;
        }
        
    }
    return 0;
}
