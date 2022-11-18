cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(apds_ctrl)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

pico_sdk_init()
include(example_auto_set_url.cmake)

add_executable(apds_ctrl main.c)

pico_generate_pio_header(apds_ctrl ${CMAKE_CURRENT_LIST_DIR}/i2c.pio)

target_sources(apds_ctrl PRIVATE pio_i2c.c pio_i2c.h APDS9960.c APDS9960.h main.c )

pico_enable_stdio_usb(apds_ctrl 1) # PICO SDK (USB ENABLED)
pico_enable_stdio_uart(apds_ctrl 0) 

target_link_libraries(apds_ctrl PRIVATE pico_stdlib hardware_pio)
pico_add_extra_outputs(apds_ctrl)

