add_executable(motor
        motor.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(motor pico_stdlib hardware_pwm)

# remap the uart communication to usb
pico_enable_stdio_usb(motor 1)
pico_enable_stdio_uart(motor 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(motor)