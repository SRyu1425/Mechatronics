cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(hw)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
# Adds example
add_subdirectory(hw2)
add_subdirectory(hw3)
add_subdirectory(hw4)
add_subdirectory(hw5)
add_subdirectory(hw6)
add_subdirectory(hw7)
add_subdirectory(hw8)
add_subdirectory(hw12)
add_subdirectory(hw13)
add_subdirectory(Robot_Proj)

