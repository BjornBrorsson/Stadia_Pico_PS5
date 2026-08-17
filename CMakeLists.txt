cmake_minimum_required(VERSION 3.13)

# Initialize Pico SDK
include(pico_sdk_import.cmake)

project(stadia_ps5_bridge C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

# Set default board to Raspberry Pi Pico W / WH
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Initialize the Pico SDK
pico_sdk_init()

add_executable(stadia_ps5_bridge
    src/main.c
    src/stadia_ble_client.c
    src/usb_descriptors.c
    src/usb_gamepad.c
    src/input_mapper.c
    src/status_led.c
)

# Include search directories
target_include_directories(stadia_ps5_bridge PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Link libraries
# pico_cyw43_arch_none + pico_btstack_ble + pico_btstack_cyw43 provides native BTstack integration
# tinyusb_device provides USB controller emulation
# hardware_flash / hardware_sync provides non-volatile profile persistence
target_link_libraries(stadia_ps5_bridge PRIVATE
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_ble
    pico_btstack_cyw43
    tinyusb_device
    tinyusb_board
    hardware_flash
    hardware_sync
)

# Enable USB UART output for debugging (optional) and disable default standard USB CDC to avoid endpoint conflict
pico_enable_stdio_usb(stadia_ps5_bridge 0)
pico_enable_stdio_uart(stadia_ps5_bridge 1)

# Generate UF2, HEX, and BIN outputs
pico_add_extra_outputs(stadia_ps5_bridge)
