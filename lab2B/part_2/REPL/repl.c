#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "neopixel.h"
#include "hardware/gpio.h"

#define QTPY_BOOT_PIN 21
#define LED_CTRL_PIN 29

// sio base is 0xd0000000
// gpio input offset is 0x00000004
// full register address is 0xd0000004

typedef struct
{
    long timeStamp;
    uint32_t target_address;
    uint32_t address_content;
    uint32_t write_content;
    uint32_t mask_value;
    bool isRead;
    bool isAddress;
    bool isMask;
    char addressChars[8];
    char contentChars[8];
} ReplStatus;

void render_to_console(ReplStatus status, char inputChars[], ReplStatus prevStatus)
{
    if (true)//prevStatus.address_content != status.address_content)
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
    stdio_init_all();
    gpio_init(QTPY_BOOT_PIN);
    gpio_set_dir(QTPY_BOOT_PIN, 0);
    neopixel_init();

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

    prevStatus = status;

    inputchars[8] = '\0';

    while (true)
    {
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
            }
            else
            {
                // if the input is not the mode control and it is a valid hex number, then save it in the array
                if ((inputch <= '9' && inputch >= '0') || (inputch >= 'A' && inputch <= 'F') || (inputch >= 'a' && inputch <= 'f'))
                {
                    inputchars[counter] = inputch;
                    counter++;
                }
                else
                {
                    sleep_ms(10);
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
        sleep_ms(100); // don't DDOS the serial console
        status.timeStamp = status.timeStamp + 1;

        pin_29_address = (volatile uint32_t *)0xd0000010;     // the pointer of GPIO_SET register
        full_gpio_register_value = (uint32_t)*pin_29_address; // retrive the register value lies in the address
        pin_29_selection_mask = 1u << 29;                     // mask used to retrieve the 29th bit
        selected_pin_state = full_gpio_register_value & pin_29_selection_mask;
        shifted_pin_29_state = selected_pin_state >> 29; // retrive the bit value

        if (gpio_get(QTPY_BOOT_PIN) && !shifted_pin_29_state)
        {
            neopixel_set_rgb(0x00000000);
        }
        else
        {
            neopixel_set_rgb(0xffffffff);
        }

        
    }
    return 0;
}
