cmake_minimum_required(VERSION 3.13)

# Define the executable and its source files
add_executable(usb
    usb.c
    usb_descriptors.c
)

# Ensure TinyUSB can find its configuration
target_include_directories(usb PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link necessary libraries including pico_stdlib and pico_stdio_usb
target_link_libraries(usb PUBLIC
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
    pico_stdio_usb  # Ensure this is included for stdio_usb_connected
    hardware_i2c
)

# Enable USB stdio and disable UART stdio
pico_enable_stdio_usb(usb 1)
pico_enable_stdio_uart(usb 0)

# Add extra output formats (map/bin/hex)
pico_add_extra_outputs(usb)

# Optionally add RP2040 USB device enumeration fix
# Uncomment the line below if needed
# target_compile_definitions(usb PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)
