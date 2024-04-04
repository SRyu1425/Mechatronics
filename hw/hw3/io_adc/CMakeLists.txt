add_executable(io_adc
        io_adc.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(io_adc pico_stdlib hardware_adc)

# remap the uart communication to usb
pico_enable_stdio_usb(io_adc 1)
pico_enable_stdio_uart(io_adc 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(io_adc)