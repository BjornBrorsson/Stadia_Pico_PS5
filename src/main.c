#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "bsp/board.h"
#include "config.h"
#include "status_led.h"
#include "input_mapper.h"
#include "usb_gamepad.h"
#include "stadia_ble_client.h"

// Latest received state
static gamepad_state_t latest_gamepad_state;
static uint32_t reset_button_hold_start = 0;

// Callback from BLE client when a new packet arrives from Stadia controller
static void on_stadia_state_received(const gamepad_state_t *state) {
    if (!state) return;
    memcpy(&latest_gamepad_state, state, sizeof(gamepad_state_t));

    // Forward to USB host (Brook Wingman P5 / PS5 / PC)
    usb_gamepad_send_state(&latest_gamepad_state);

    // Check for pairing reset combo: Assistant (●) + Capture ([ ]) held for 3 seconds
    if ((state->buttons & BTN_MASK_TOUCHPAD_ASSIST) && (state->buttons & BTN_MASK_CAPTURE_MUTE)) {
        if (reset_button_hold_start == 0) {
            reset_button_hold_start = to_ms_since_boot(get_absolute_time());
        } else if (to_ms_since_boot(get_absolute_time()) - reset_button_hold_start > 3000) {
            // Trigger unpair and re-scan
            stadia_ble_clear_bonding_and_rescan();
            reset_button_hold_start = 0;
        }
    } else {
        reset_button_hold_start = 0;
    }
}

// Callback from USB host when rumble data arrives
static void on_host_rumble_received(uint8_t low_freq, uint8_t high_freq) {
    stadia_ble_send_rumble(low_freq, high_freq);
}

int main(void) {
    // 1. Initialize Board & Standard I/O
    board_init();
    stdio_init_all();

    // 2. Initialize CYW43 Wireless / Bluetooth Chip
    if (cyw43_arch_init()) {
        printf("ERROR: Failed to initialize CYW43 architecture\n");
        return -1;
    }

    // 3. Initialize Status LED Engine
    status_led_init();

    // 4. Initialize Profile & Input Mapper (Reads profile from Flash memory)
    controller_profile_t active_profile = input_mapper_init();

    // 5. Initialize USB Gamepad Emulation
    usb_gamepad_init(active_profile, on_host_rumble_received);

    // 6. Initialize BTstack BLE Central Client (Scans & Connects to Stadia Controller)
    stadia_ble_init(on_stadia_state_received);

    printf("Stadia Controller to PS5/Wingman Bridge started.\n");
    printf("Active Profile: %d\n", active_profile);

    // 7. Main Cooperative Execution Loop
    while (1) {
        // Run TinyUSB tasks
        usb_gamepad_task();

        // Run Status LED tasks
        status_led_task();

        // Feed background network & BTstack events
        cyw43_arch_poll();
    }

    return 0;
}
