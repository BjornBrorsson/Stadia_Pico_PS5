#include "usb_gamepad.h"
#include "usb_descriptors.h"
#include "input_mapper.h"
#include "status_led.h"
#include "bsp/board.h"
#include <string.h>

static controller_profile_t current_profile = PROFILE_XINPUT_XBOX360;
static host_rumble_callback_t rumble_callback = NULL;
static bool is_suspended = false;

void usb_gamepad_init(controller_profile_t profile, host_rumble_callback_t rumble_cb) {
    current_profile = profile;
    rumble_callback = rumble_cb;
    usb_descriptors_set_profile(profile);
    tusb_init();
}

void usb_gamepad_task(void) {
    tud_task();

    // Check for incoming XInput OUT packets (Rumble / LED data)
    if (current_profile == PROFILE_XINPUT_XBOX360 && tud_vendor_available()) {
        uint8_t out_buf[32];
        uint32_t count = tud_vendor_read(out_buf, sizeof(out_buf));
        if (count >= 8 && out_buf[0] == 0x00 && out_buf[1] == 0x08) {
            // XInput Rumble Packet: [0x00, 0x08, 0x00, LeftMotor, RightMotor, 0, 0, 0]
            uint8_t left_rumble = out_buf[3];
            uint8_t right_rumble = out_buf[4];
            if (rumble_callback) {
                rumble_callback(left_rumble, right_rumble);
            }
        }
    }
}

bool usb_gamepad_is_mounted(void) {
    return tud_mounted() && !is_suspended;
}

void usb_gamepad_send_state(const gamepad_state_t *state) {
    if (!usb_gamepad_is_mounted() || !state) return;

    if (current_profile == PROFILE_XINPUT_XBOX360) {
        if (tud_vendor_mounted() && tud_vendor_write_available() >= sizeof(xinput_report_t)) {
            xinput_report_t xreport;
            input_mapper_build_xinput(state, &xreport);
            tud_vendor_write(&xreport, sizeof(xinput_report_t));
            tud_vendor_flush();
            status_led_trigger_activity();
        }
    } else if (current_profile == PROFILE_PS4_DUALSHOCK4) {
        if (tud_hid_ready()) {
            ds4_report_t ds4;
            input_mapper_build_ds4(state, &ds4);
            tud_hid_report(0, &ds4, sizeof(ds4_report_t));
            status_led_trigger_activity();
        }
    } else if (current_profile == PROFILE_PS5_DUALSENSE) {
        if (tud_hid_ready()) {
            dualsense_report_t ds5;
            input_mapper_build_dualsense(state, &ds5);
            tud_hid_report(0, &ds5, sizeof(dualsense_report_t));
            status_led_trigger_activity();
        }
    } else if (current_profile == PROFILE_SWITCH_PRO) {
        if (tud_hid_ready()) {
            switch_pro_report_t sw;
            input_mapper_build_switch(state, &sw);
            tud_hid_report(0, &sw, sizeof(switch_pro_report_t));
            status_led_trigger_activity();
        }
    } else if (current_profile == PROFILE_PS3_DINPUT) {
        if (tud_hid_ready()) {
            ps3_report_t ps3;
            input_mapper_build_ps3(state, &ps3);
            tud_hid_report(0, &ps3, sizeof(ps3_report_t));
            status_led_trigger_activity();
        }
    }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
    is_suspended = false;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    is_suspended = false;
}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    is_suspended = true;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    is_suspended = false;
}

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

// Invoked when received SET_REPORT control request or received data on OUT endpoint (HID Rumble)
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;
    (void) report_type;

    if (!buffer || bufsize == 0) return;

    if (current_profile == PROFILE_PS4_DUALSHOCK4 || current_profile == PROFILE_PS5_DUALSENSE) {
        // DS4 / DualSense Output Report (Report ID 0x05 or 0x01):
        // Offset 4: Right Motor (small / high frequency)
        // Offset 5: Left Motor (large / low frequency)
        if (bufsize >= 6) {
            uint8_t high_freq = buffer[4];
            uint8_t low_freq  = buffer[5];
            if (rumble_callback) {
                rumble_callback(low_freq, high_freq);
            }
        }
    } else if (current_profile == PROFILE_SWITCH_PRO) {
        // Switch Pro Rumble Packet
        if (bufsize >= 4) {
            uint8_t low_freq  = buffer[1];
            uint8_t high_freq = buffer[2];
            if (rumble_callback) {
                rumble_callback(low_freq, high_freq);
            }
        }
    }
}
