cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(apds_C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})
pico_sdk_init()
include(example_auto_set_url.cmake)

add_executable(apds_C main.c)

pico_generate_pio_header(apds_C ${CMAKE_CURRENT_LIST_DIR}/i2c.pio)
pico_generate_pio_header(apds_C ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_sources(apds_C PRIVATE
        pio_i2c.c
        pio_i2c.h
        APDS9960.c
        APDS9960.h
        ws2812.h
        ws2812.c
        main.c
        )

pico_enable_stdio_usb(apds_C 1) 
pico_enable_stdio_uart(apds_C 0) 

target_link_libraries(apds_C PRIVATE pico_stdlib hardware_pio)
pico_add_extra_outputs(apds_C)

