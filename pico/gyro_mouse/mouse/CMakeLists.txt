add_executable(mouse
        mouse.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(mouse pico_stdlib hardware_pwm hardware_i2c)


# remap the uart communication to usb
pico_enable_stdio_usb(mouse 1)
pico_enable_stdio_uart(mouse 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(mouse)