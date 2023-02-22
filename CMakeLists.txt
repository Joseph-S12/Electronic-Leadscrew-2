cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH "/usr/share/pico-sdk")

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Electronic-Leadscrew C CXX ASM)

pico_sdk_init()

add_subdirectory(leadscrew)
