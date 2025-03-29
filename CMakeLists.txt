cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pacdrive_rp2040 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(${PROJECT_NAME} 
main.c
usb_descriptors.c
)
target_include_directories(
    ${PROJECT_NAME} 
    PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME} 
PRIVATE 
pico_stdlib
tinyusb_device
tinyusb_board
)

pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_uart(${PROJECT_NAME} 1)

# add flash target (make flash)
add_custom_target(flash DEPENDS ${PROJECT_NAME}.elf)
add_custom_command(TARGET flash
    USES_TERMINAL
    COMMAND openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${PROJECT_NAME}.elf verify reset exit")
add_custom_target(pflash DEPENDS ${PROJECT_NAME}.uf2)
    add_custom_command(TARGET pflash
        USES_TERMINAL
        COMMAND picotool load ${PROJECT_NAME}.uf2 -f && picotool reboot)