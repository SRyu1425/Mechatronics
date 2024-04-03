add_executable(blink
        blink.c
        )

# pull in common dependencies
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(blink pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(blink)