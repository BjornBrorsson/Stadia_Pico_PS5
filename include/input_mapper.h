#ifndef _INPUT_MAPPER_H_
#define _INPUT_MAPPER_H_

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "usb_gamepad.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initializes input mapping engine & loads profile from Flash
controller_profile_t input_mapper_init(void);

// Save selected profile to non-volatile RP2040 Flash
bool input_mapper_save_profile(controller_profile_t profile);

// Unpacks raw Stadia BLE Report ID 0x03 packet into normalized gamepad_state_t
bool input_mapper_parse_stadia_report(const uint8_t *data, uint16_t len, gamepad_state_t *out_state);

// Converts normalized gamepad_state_t into XInput report
void input_mapper_build_xinput(const gamepad_state_t *state, xinput_report_t *out_report);

// Converts normalized gamepad_state_t into PS4 DualShock 4 report
void input_mapper_build_ds4(const gamepad_state_t *state, ds4_report_t *out_report);

// Converts normalized gamepad_state_t into Nintendo Switch Pro report
void input_mapper_build_switch(const gamepad_state_t *state, switch_pro_report_t *out_report);

// Converts normalized gamepad_state_t into PS5 DualSense report
void input_mapper_build_dualsense(const gamepad_state_t *state, dualsense_report_t *out_report);

// Converts normalized gamepad_state_t into PS3 DualShock 3 report
void input_mapper_build_ps3(const gamepad_state_t *state, ps3_report_t *out_report);

// Helper to convert D-Pad bitmask into 0-7 Hat switch direction (8 = Centered)
uint8_t input_mapper_dpad_to_hat(uint8_t dpad_mask);

// Helper to convert 0-7 Hat switch into D-Pad bitmask
uint8_t input_mapper_hat_to_dpad(uint8_t hat);

#ifdef __cplusplus
}
#endif

#endif /* _INPUT_MAPPER_H_ */
