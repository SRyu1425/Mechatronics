add_executable(servo
        servo.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(servo pico_stdlib hardware_pwm)

# create map/bin/hex file etc.
pico_add_extra_outputs(servo)