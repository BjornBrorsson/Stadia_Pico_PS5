#include "input_mapper.h"
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// Flash storage address for profile configuration (last 4KB sector of 2MB flash)
#define FLASH_TARGET_OFFSET (1024 * 1024 + 512 * 1024) // 1.5MB offset
static const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

static controller_profile_t active_profile = PROFILE_XINPUT_XBOX360;

// Convert Stadia 8-bit unsigned stick (0..255, 128=center) to int16_t (-32768..32767)
static inline int16_t scale_axis(uint8_t raw_val, bool invert) {
    int32_t val = ((int32_t)raw_val - 128);
    // Apply small deadzone around center
    if (val > -6 && val < 6) {
        val = 0;
    }
    int32_t scaled = val * 256;
    if (scaled > 32767) scaled = 32767;
    if (scaled < -32768) scaled = -32768;
    return invert ? (int16_t)(-scaled) : (int16_t)scaled;
}

controller_profile_t input_mapper_init(void) {
    const persistent_config_t *saved_config = (const persistent_config_t *)flash_target_contents;
    if (saved_config->magic == FLASH_CONFIG_MAGIC && saved_config->active_profile < PROFILE_COUNT) {
        active_profile = (controller_profile_t)saved_config->active_profile;
    } else {
        active_profile = PROFILE_XINPUT_XBOX360; // Default: Xbox 360 for Brook Wingman P5
    }
    return active_profile;
}

bool input_mapper_save_profile(controller_profile_t profile) {
    if (profile >= PROFILE_COUNT) return false;
    active_profile = profile;

    persistent_config_t new_config;
    memset(&new_config, 0, sizeof(new_config));
    new_config.magic = FLASH_CONFIG_MAGIC;
    new_config.active_profile = (uint8_t)profile;

    uint8_t buffer[FLASH_PAGE_SIZE];
    memset(buffer, 0xFF, sizeof(buffer));
    memcpy(buffer, &new_config, sizeof(new_config));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    restore_interrupts(ints);

    return true;
}

uint8_t input_mapper_hat_to_dpad(uint8_t hat) {
    switch (hat) {
        case 0: return DPAD_MASK_UP;
        case 1: return DPAD_MASK_UP | DPAD_MASK_RIGHT;
        case 2: return DPAD_MASK_RIGHT;
        case 3: return DPAD_MASK_DOWN | DPAD_MASK_RIGHT;
        case 4: return DPAD_MASK_DOWN;
        case 5: return DPAD_MASK_DOWN | DPAD_MASK_LEFT;
        case 6: return DPAD_MASK_LEFT;
        case 7: return DPAD_MASK_UP | DPAD_MASK_LEFT;
        default: return 0; // Centered
    }
}

uint8_t input_mapper_dpad_to_hat(uint8_t dpad_mask) {
    bool up    = (dpad_mask & DPAD_MASK_UP) != 0;
    bool down  = (dpad_mask & DPAD_MASK_DOWN) != 0;
    bool left  = (dpad_mask & DPAD_MASK_LEFT) != 0;
    bool right = (dpad_mask & DPAD_MASK_RIGHT) != 0;

    if (up && right) return 1;
    if (down && right) return 3;
    if (down && left) return 5;
    if (up && left) return 7;
    if (up) return 0;
    if (right) return 2;
    if (down) return 4;
    if (left) return 6;
    return 8; // Centered
}

bool input_mapper_parse_stadia_report(const uint8_t *data, uint16_t len, gamepad_state_t *out_state) {
    if (!data || !out_state || len < 9) return false;

    const uint8_t *report_bytes = data;
    uint16_t report_len = len;

    // Check if Report ID 0x03 is present as prefix (e.g. 10 or 11 byte report where data[1] is hat 0..8)
    if (data[0] == 0x03 && len >= 10 && data[1] <= 8) {
        report_bytes = &data[1];
        report_len = len - 1;
    }

    if (report_len < 9) return false;

    out_state->connected = true;
    out_state->buttons = 0;
    out_state->dpad = input_mapper_hat_to_dpad(report_bytes[0]);

    // Parse Byte 1 (System, Sticks, System Buttons)
    uint8_t b2 = report_bytes[1];
    if (b2 & 0x01) out_state->buttons |= BTN_MASK_CAPTURE_MUTE;
    if (b2 & 0x02) out_state->buttons |= BTN_MASK_TOUCHPAD_ASSIST;
    if (b2 & 0x10) out_state->buttons |= BTN_MASK_HOME_GUIDE;
    if (b2 & 0x20) out_state->buttons |= BTN_MASK_START_OPTIONS;
    if (b2 & 0x40) out_state->buttons |= BTN_MASK_SELECT_SHARE;
    if (b2 & 0x80) out_state->buttons |= BTN_MASK_R3;

    // Parse Byte 2 (Face buttons, Bumpers, L3)
    uint8_t b3 = report_bytes[2];
    if (b3 & 0x01) out_state->buttons |= BTN_MASK_L3;
    if (b3 & 0x02) out_state->buttons |= BTN_MASK_R1;
    if (b3 & 0x04) out_state->buttons |= BTN_MASK_L1;
    if (b3 & 0x08) out_state->buttons |= BTN_MASK_TRIANGLE_Y;
    if (b3 & 0x10) out_state->buttons |= BTN_MASK_SQUARE_X;
    if (b3 & 0x20) out_state->buttons |= BTN_MASK_CIRCLE_B;
    if (b3 & 0x40) out_state->buttons |= BTN_MASK_CROSS_A;

    // Parse Analog Sticks (scaled to -32768..32767)
    out_state->stick_lx = scale_axis(report_bytes[3], false);
    out_state->stick_ly = scale_axis(report_bytes[4], true);  // Inverted so Up is positive for standard 3D math
    out_state->stick_rx = scale_axis(report_bytes[5], false);
    out_state->stick_ry = scale_axis(report_bytes[6], true);

    // Parse Analog Triggers (0..255)
    out_state->trigger_l = report_bytes[7];
    out_state->trigger_r = report_bytes[8];

    // Digital threshold for trigger clicks
    if (out_state->trigger_l > 40) out_state->buttons |= BTN_MASK_L2_DIGITAL;
    if (out_state->trigger_r > 40) out_state->buttons |= BTN_MASK_R2_DIGITAL;

    if (report_len >= 10) {
        out_state->battery_level = report_bytes[9];
    } else {
        out_state->battery_level = 100;
    }

    return true;
}

void input_mapper_build_xinput(const gamepad_state_t *state, xinput_report_t *out) {
    memset(out, 0, sizeof(xinput_report_t));
    out->report_id = 0x00;
    out->report_size = sizeof(xinput_report_t);

    uint16_t xbtn = 0;
    if (state->dpad & DPAD_MASK_UP)    xbtn |= XINPUT_MASK_DPAD_UP;
    if (state->dpad & DPAD_MASK_DOWN)  xbtn |= XINPUT_MASK_DPAD_DOWN;
    if (state->dpad & DPAD_MASK_LEFT)  xbtn |= XINPUT_MASK_DPAD_LEFT;
    if (state->dpad & DPAD_MASK_RIGHT) xbtn |= XINPUT_MASK_DPAD_RIGHT;

    if (state->buttons & BTN_MASK_START_OPTIONS)  xbtn |= XINPUT_MASK_START;
    if (state->buttons & BTN_MASK_SELECT_SHARE)   xbtn |= XINPUT_MASK_BACK;
    if (state->buttons & BTN_MASK_L3)             xbtn |= XINPUT_MASK_LS;
    if (state->buttons & BTN_MASK_R3)             xbtn |= XINPUT_MASK_RS;
    if (state->buttons & BTN_MASK_L1)             xbtn |= XINPUT_MASK_LB;
    if (state->buttons & BTN_MASK_R1)             xbtn |= XINPUT_MASK_RB;
    if (state->buttons & BTN_MASK_HOME_GUIDE)     xbtn |= XINPUT_MASK_GUIDE;
    if (state->buttons & BTN_MASK_CROSS_A)        xbtn |= XINPUT_MASK_A;
    if (state->buttons & BTN_MASK_CIRCLE_B)       xbtn |= XINPUT_MASK_B;
    if (state->buttons & BTN_MASK_SQUARE_X)       xbtn |= XINPUT_MASK_X;
    if (state->buttons & BTN_MASK_TRIANGLE_Y)     xbtn |= XINPUT_MASK_Y;

    // Map Assistant and Capture to extra actions (e.g. Back/Guide or Stick Clicks)
    if (state->buttons & BTN_MASK_TOUCHPAD_ASSIST) xbtn |= XINPUT_MASK_BACK; // Touchpad click mapped to Back on Xbox
    if (state->buttons & BTN_MASK_CAPTURE_MUTE)    xbtn |= XINPUT_MASK_GUIDE;

    out->buttons = xbtn;
    out->left_trigger  = state->trigger_l;
    out->right_trigger = state->trigger_r;
    out->thumb_lx = state->stick_lx;
    out->thumb_ly = state->stick_ly; // Up is positive in XInput
    out->thumb_rx = state->stick_rx;
    out->thumb_ry = state->stick_ry;
}

void input_mapper_build_ds4(const gamepad_state_t *state, ds4_report_t *out) {
    memset(out, 0, sizeof(ds4_report_t));
    out->report_id = 0x01;

    // Convert sticks from -32768..32767 back to 0..255 (128 center, Y down is +255 in DS4)
    out->lx = (uint8_t)((state->stick_lx >> 8) + 128);
    out->ly = (uint8_t)(((-state->stick_ly) >> 8) + 128);
    out->rx = (uint8_t)((state->stick_rx >> 8) + 128);
    out->ry = (uint8_t)(((-state->stick_ry) >> 8) + 128);

    uint8_t hat = input_mapper_dpad_to_hat(state->dpad);
    out->dpad_buttons = (hat & 0x0F);

    if (state->buttons & BTN_MASK_SQUARE_X)   out->dpad_buttons |= (1 << 4);
    if (state->buttons & BTN_MASK_CROSS_A)    out->dpad_buttons |= (1 << 5);
    if (state->buttons & BTN_MASK_CIRCLE_B)   out->dpad_buttons |= (1 << 6);
    if (state->buttons & BTN_MASK_TRIANGLE_Y) out->dpad_buttons |= (1 << 7);

    uint8_t b1 = 0;
    if (state->buttons & BTN_MASK_L1)           b1 |= (1 << 0);
    if (state->buttons & BTN_MASK_R1)           b1 |= (1 << 1);
    if (state->buttons & BTN_MASK_L2_DIGITAL)   b1 |= (1 << 2);
    if (state->buttons & BTN_MASK_R2_DIGITAL)   b1 |= (1 << 3);
    if (state->buttons & BTN_MASK_SELECT_SHARE)  b1 |= (1 << 4); // Share
    if (state->buttons & BTN_MASK_START_OPTIONS) b1 |= (1 << 5); // Options
    if (state->buttons & BTN_MASK_L3)           b1 |= (1 << 6);
    if (state->buttons & BTN_MASK_R3)           b1 |= (1 << 7);
    out->buttons_1 = b1;

    uint8_t b2 = 0;
    if (state->buttons & BTN_MASK_HOME_GUIDE)      b2 |= (1 << 0); // PS Button
    if (state->buttons & BTN_MASK_TOUCHPAD_ASSIST) b2 |= (1 << 1); // Touchpad Click
    out->buttons_2 = b2;

    out->l2_analog = state->trigger_l;
    out->r2_analog = state->trigger_r;
    out->battery = 0x1B; // Fully charged, cable connected
}

void input_mapper_build_dualsense(const gamepad_state_t *state, dualsense_report_t *out) {
    memset(out, 0, sizeof(dualsense_report_t));
    out->report_id = 0x01;

    out->lx = (uint8_t)((state->stick_lx >> 8) + 128);
    out->ly = (uint8_t)(((-state->stick_ly) >> 8) + 128);
    out->rx = (uint8_t)((state->stick_rx >> 8) + 128);
    out->ry = (uint8_t)(((-state->stick_ry) >> 8) + 128);
    out->l2_analog = state->trigger_l;
    out->r2_analog = state->trigger_r;

    uint8_t hat = input_mapper_dpad_to_hat(state->dpad);
    out->dpad_buttons = (hat & 0x0F);

    if (state->buttons & BTN_MASK_SQUARE_X)   out->dpad_buttons |= (1 << 4);
    if (state->buttons & BTN_MASK_CROSS_A)    out->dpad_buttons |= (1 << 5);
    if (state->buttons & BTN_MASK_CIRCLE_B)   out->dpad_buttons |= (1 << 6);
    if (state->buttons & BTN_MASK_TRIANGLE_Y) out->dpad_buttons |= (1 << 7);

    uint8_t b1 = 0;
    if (state->buttons & BTN_MASK_L1)           b1 |= (1 << 0);
    if (state->buttons & BTN_MASK_R1)           b1 |= (1 << 1);
    if (state->buttons & BTN_MASK_L2_DIGITAL)   b1 |= (1 << 2);
    if (state->buttons & BTN_MASK_R2_DIGITAL)   b1 |= (1 << 3);
    if (state->buttons & BTN_MASK_SELECT_SHARE)  b1 |= (1 << 4); // Create
    if (state->buttons & BTN_MASK_START_OPTIONS) b1 |= (1 << 5); // Options
    if (state->buttons & BTN_MASK_L3)           b1 |= (1 << 6);
    if (state->buttons & BTN_MASK_R3)           b1 |= (1 << 7);
    out->buttons_1 = b1;

    uint8_t b2 = 0;
    if (state->buttons & BTN_MASK_HOME_GUIDE)      b2 |= (1 << 0); // PS Button
    if (state->buttons & BTN_MASK_TOUCHPAD_ASSIST) b2 |= (1 << 1); // Touchpad Click
    if (state->buttons & BTN_MASK_CAPTURE_MUTE)    b2 |= (1 << 2); // Mute button
    out->buttons_2 = b2;
}

void input_mapper_build_switch(const gamepad_state_t *state, switch_pro_report_t *out) {
    memset(out, 0, sizeof(switch_pro_report_t));
    out->report_id = 0x30;
    out->battery_conn = 0x80 | 0x0E; // USB powered + full charge

    // Switch Face Buttons (Nintendo layout)
    out->btn_b = (state->buttons & BTN_MASK_CROSS_A) ? 1 : 0;
    out->btn_a = (state->buttons & BTN_MASK_CIRCLE_B) ? 1 : 0;
    out->btn_y = (state->buttons & BTN_MASK_SQUARE_X) ? 1 : 0;
    out->btn_x = (state->buttons & BTN_MASK_TRIANGLE_Y) ? 1 : 0;

    out->btn_l = (state->buttons & BTN_MASK_L1) ? 1 : 0;
    out->btn_r = (state->buttons & BTN_MASK_R1) ? 1 : 0;
    out->btn_zl = (state->buttons & BTN_MASK_L2_DIGITAL) ? 1 : 0;
    out->btn_zr = (state->buttons & BTN_MASK_R2_DIGITAL) ? 1 : 0;

    out->btn_minus = (state->buttons & BTN_MASK_SELECT_SHARE) ? 1 : 0;
    out->btn_plus  = (state->buttons & BTN_MASK_START_OPTIONS) ? 1 : 0;
    out->btn_home  = (state->buttons & BTN_MASK_HOME_GUIDE) ? 1 : 0;
    out->btn_capture = (state->buttons & (BTN_MASK_CAPTURE_MUTE | BTN_MASK_TOUCHPAD_ASSIST)) ? 1 : 0;
    out->btn_l3    = (state->buttons & BTN_MASK_L3) ? 1 : 0;
    out->btn_r3    = (state->buttons & BTN_MASK_R3) ? 1 : 0;

    out->btn_dpad_up    = (state->dpad & DPAD_MASK_UP) ? 1 : 0;
    out->btn_dpad_down  = (state->dpad & DPAD_MASK_DOWN) ? 1 : 0;
    out->btn_dpad_left  = (state->dpad & DPAD_MASK_LEFT) ? 1 : 0;
    out->btn_dpad_right = (state->dpad & DPAD_MASK_RIGHT) ? 1 : 0;

    // Pack 12-bit analog sticks (0..4095, 2048 center)
    uint16_t lx12 = (uint16_t)(((state->stick_lx + 32768) * 4095) / 65535);
    uint16_t ly12 = (uint16_t)(((state->stick_ly + 32768) * 4095) / 65535);
    out->left_stick[0] = (uint8_t)(lx12 & 0xFF);
    out->left_stick[1] = (uint8_t)(((lx12 >> 8) & 0x0F) | ((ly12 & 0x0F) << 4));
    out->left_stick[2] = (uint8_t)((ly12 >> 4) & 0xFF);

    uint16_t rx12 = (uint16_t)(((state->stick_rx + 32768) * 4095) / 65535);
    uint16_t ry12 = (uint16_t)(((state->stick_ry + 32768) * 4095) / 65535);
    out->right_stick[0] = (uint8_t)(rx12 & 0xFF);
    out->right_stick[1] = (uint8_t)(((rx12 >> 8) & 0x0F) | ((ry12 & 0x0F) << 4));
    out->right_stick[2] = (uint8_t)((ry12 >> 4) & 0xFF);
}

void input_mapper_build_ps3(const gamepad_state_t *state, ps3_report_t *out) {
    memset(out, 0, sizeof(ps3_report_t));
    out->report_id = 0x01;

    out->btn_select   = (state->buttons & BTN_MASK_SELECT_SHARE) ? 1 : 0;
    out->btn_start    = (state->buttons & BTN_MASK_START_OPTIONS) ? 1 : 0;
    out->btn_l3       = (state->buttons & BTN_MASK_L3) ? 1 : 0;
    out->btn_r3       = (state->buttons & BTN_MASK_R3) ? 1 : 0;
    out->btn_l1       = (state->buttons & BTN_MASK_L1) ? 1 : 0;
    out->btn_r1       = (state->buttons & BTN_MASK_R1) ? 1 : 0;
    out->btn_l2       = (state->buttons & BTN_MASK_L2_DIGITAL) ? 1 : 0;
    out->btn_r2       = (state->buttons & BTN_MASK_R2_DIGITAL) ? 1 : 0;
    out->btn_cross    = (state->buttons & BTN_MASK_CROSS_A) ? 1 : 0;
    out->btn_circle   = (state->buttons & BTN_MASK_CIRCLE_B) ? 1 : 0;
    out->btn_square   = (state->buttons & BTN_MASK_SQUARE_X) ? 1 : 0;
    out->btn_triangle = (state->buttons & BTN_MASK_TRIANGLE_Y) ? 1 : 0;
    out->btn_ps       = (state->buttons & BTN_MASK_HOME_GUIDE) ? 1 : 0;

    out->btn_up    = (state->dpad & DPAD_MASK_UP) ? 1 : 0;
    out->btn_down  = (state->dpad & DPAD_MASK_DOWN) ? 1 : 0;
    out->btn_left  = (state->dpad & DPAD_MASK_LEFT) ? 1 : 0;
    out->btn_right = (state->dpad & DPAD_MASK_RIGHT) ? 1 : 0;

    out->lx = (uint8_t)((state->stick_lx >> 8) + 128);
    out->ly = (uint8_t)(((-state->stick_ly) >> 8) + 128);
    out->rx = (uint8_t)((state->stick_rx >> 8) + 128);
    out->ry = (uint8_t)(((-state->stick_ry) >> 8) + 128);

    out->l2_pressure = state->trigger_l;
    out->r2_pressure = state->trigger_r;
}
