#include "APDS9960.h"
#include "pio_i2c.h"
#include "ws2812.h"

#define SDA_PIN_BASE 22

const int addr = 0x39;
const int max_read = 250;


struct Status{
    int32_t red;
    int32_t green;
    int32_t blue;
    int32_t clear;
    int32_t prox;
};

void readInfo(struct Status* stat, PIO pio, uint sm, uint8_t addr, bool nostop){
    read_value(&stat->prox, &stat->red, &stat->green, &stat->blue, &stat->clear, pio, sm, addr, nostop);
    printf("Proximity: %d, clear: %d, red: %d, green: %d, blue: %d\n", stat->prox, stat->clear, stat->red, stat->green, stat->blue);
}

int main(){
    stdio_init_all();
    sleep_ms(5000);
    printf("Initialization Starts\n");

    PIO pio_0 = pio0;
    uint sm_0 = 0;

    uint offset_0 = pio_add_program(pio_0, &i2c_program);
    i2c_program_init(pio_0, sm_0, offset_0, SDA_PIN_BASE);
    
    APDS9960_init(pio_0, sm_0, addr, false);

    const uint POWER_PIN = 11;
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, GPIO_OUT);
    gpio_put(POWER_PIN,1);
    
    PIO pio = pio1;
    int sm = 1;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, 12, 800000, true);

    printf("Initialization Finished\n");
    
    struct Status status;
    while(true){
        sleep_ms(1000);
        readInfo(&status, pio_0, sm_0, addr, false);
        set_neopixel_color(rgb_to_neopixel(status.red*255/4096, status.green*255/4096, status.blue*255/4096));
        printf("newpixel color synchronized\n");
        printf("*********************************************\n");
    }

}