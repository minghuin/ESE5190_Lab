#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "string.h"
#include "pio_i2c.h"

const int addr = 0x39;
const int max_read = 250;

#define SDA_PIN_PASE 22


void readInfo(PIO pio, uint sm, uint8_t addr, bool nostop){
    int32_t prox;
    int32_t red;
    int32_t green;
    int32_t blue;
    int32_t clear;
    read_value(&red, &green, &blue, &clear, &prox, pio, sm, addr, nostop);
    printf("APDS: Proximity: %d, clear: %d, red: %d, green: %d, blue: %d\n", prox, clear, red, green, blue);
}

int main(){
    stdio_init_all();
    // pio init
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &i2c_program);
    i2c_program_init(pio, sm, offset, SDA_PIN_PASE);

    // APDS9960 init
    APDS9960_init(pio, sm, addr, false);

    while(true){
        sleep_ms(1000);
        readInfo(pio, sm, addr, false); 
    }
}

