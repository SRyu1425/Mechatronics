add_executable(oled
        oled.c
        ssd1306.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(oled pico_stdlib hardware_i2c hardware_adc)

# remap the uart communication to usb
pico_enable_stdio_usb(oled 1)
pico_enable_stdio_uart(oled 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(oled)