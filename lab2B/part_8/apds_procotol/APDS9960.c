#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "string.h"
#include "APDS9960.h"
#include "pio_i2c.h"


#define POWER_ENABLE 1
#define PROXIMITY_ENABLE 1
#define ALS_ENABLE 1
#define GESTURE_ENABLE 0

#define ALS_GAIN 0
#define ALS_TIME 219

#define INIT_CONFIG (GESTURE_ENABLE << 6u) | (PROXIMITY_ENABLE << 2u) | (ALS_ENABLE << 1u) | POWER_ENABLE



void APDS9960_init(PIO pio, uint sm,uint8_t addr, bool nostop) {

    // initialize the APDS9960 based on the datasheet
    int len = 2;
    uint8_t config[2];
    config[0] = ENABLE_REG;
    config[1] = INIT_CONFIG;
    pio_i2c_write_blocking(pio, sm, addr, config, len, false);
    config[0] = ALS_TIME_REG;
    config[1] = ALS_TIME;
    pio_i2c_write_blocking(pio, sm, addr, config, len, false);
}

void read_value(int32_t* red, int32_t* green, int32_t* blue, int32_t* clear, int32_t* prox, PIO pio, uint sm, uint8_t addr, bool nostop) {

    uint8_t buf[1];
    uint8_t reg = PDATA_REG;
    pio_i2c_write_blocking(pio, sm, addr, &reg, 1, nostop);  
    pio_i2c_read_blocking(pio, sm, addr, buf, 1); 

    *prox = buf[0];

    uint8_t buffer[2];
    reg = CDATA_REG_L;
    pio_i2c_write_blocking(pio, sm, addr, &reg, 1, nostop);  
    pio_i2c_read_blocking(pio, sm, addr, buffer, 8); 

    *clear = (buffer[1] << 8) | buffer[0];
    *red = (buffer[3] << 8) | buffer[2];
    *green = (buffer[5] << 8) | buffer[4];
    *blue = (buffer[7] << 8) | buffer[6];
}
