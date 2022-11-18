#include "ws2812.h"

#define IS_RGBW true
#define NUM_PIXELS 150

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 11 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 11
#endif

void set_neopixel_color(uint32_t rgb){
    uint32_t grb=((rgb&0xff0000) >>8) | ((rgb&0x00ff00)<<8) | ((rgb&0x0000ff));
    printf("grb value: 0x%08x \n", grb);
    pio_sm_put_blocking(pio1, 1, grb << 8u);
}

uint32_t rgb_to_neopixel(uint8_t r, uint8_t g, uint8_t b){

    uint32_t result = 0;
    result |= (uint32_t)r << 16u;
    result |= (uint32_t)g << 8u;
    result |= (uint32_t)b;
    
    return result;
}
