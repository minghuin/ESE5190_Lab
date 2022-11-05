# Task 1 Flashlight

For this task, I changed the `gpio_get(QTPY_BOOT_PIN)` function to 5 lines of C code to implement the direct access of the register of GPIO_IN and extract the PIN21's value. Then, this value is used to control the neopixel module.