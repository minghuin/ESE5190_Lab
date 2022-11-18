# Part 8 APDS Control

In this part, we implemented the APDS9960 protocol and could read data from the sensor by a C program.

To be more specific, we used and modified the pio_i2c example offered by the pico-example repo to implement the i2c communication through pio module.



The APDS9960.c has two parts:

1. Initialization

```c
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
```

The array config contains the information to reserve the register address and the initialization configuration of the senser. The `pio_i2c_write_blocking` function write a block to the APDS to initialize the APDS9960

2. Read data from the sensor

```C
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

```

This function read the data by utilizing the functions of `pio_i2c_read_blocking` and `pio_i2c_write_blocking`. According to the datasheet of the APDS9960, the proximity data and the color data are transmitted by 2 adjacent blocks. That's what we have done in the program.



Finally, we could use of main function to read data from the APDS9960 and print the data in the console as the gif [here](https://github.com/minghuin/ESE5190_Lab/blob/main/lab2B/part_8/part8.gif)







