add_executable(complete
        complete.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(complete pico_stdlib hardware_pwm hardware_uart hardware_irq)

# remap the uart communication to usb
pico_enable_stdio_usb(complete 1)
pico_enable_stdio_uart(complete 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(complete)