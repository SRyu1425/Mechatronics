add_executable(uart
        uart.c
        ssd1306.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(uart pico_stdlib hardware_uart hardware_i2c)

# remap the uart communication to usb
pico_enable_stdio_usb(uart 1)
pico_enable_stdio_uart(uart 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(uart)