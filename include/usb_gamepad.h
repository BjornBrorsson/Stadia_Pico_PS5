#ifndef _USB_GAMEPAD_H_
#define _USB_GAMEPAD_H_

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// Packed USB Gamepad Report Structures
//--------------------------------------------------------------------+

// 1. Microsoft Xbox 360 / XInput Report (20 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  report_id;    // 0x00
    uint8_t  report_size;  // 0x14 (20 bytes)
    uint16_t buttons;      // Bitmask of XInput buttons
    uint8_t  left_trigger; // 0..255
    uint8_t  right_trigger;// 0..255
    int16_t  thumb_lx;     // -32768..32767
    int16_t  thumb_ly;     // -32768..32767
    int16_t  thumb_rx;     // -32768..32767
    int16_t  thumb_ry;     // -32768..32767
    uint8_t  reserved[6];  // zeroes
} xinput_report_t;

// XInput Button Bitmasks
#define XINPUT_MASK_DPAD_UP     (1u << 0)
#define XINPUT_MASK_DPAD_DOWN   (1u << 1)
#define XINPUT_MASK_DPAD_LEFT   (1u << 2)
#define XINPUT_MASK_DPAD_RIGHT  (1u << 3)
#define XINPUT_MASK_START       (1u << 4)
#define XINPUT_MASK_BACK        (1u << 5)
#define XINPUT_MASK_LS          (1u << 6)
#define XINPUT_MASK_RS          (1u << 7)
#define XINPUT_MASK_LB          (1u << 8)
#define XINPUT_MASK_RB          (1u << 9)
#define XINPUT_MASK_GUIDE       (1u << 10)
#define XINPUT_MASK_UNUSED      (1u << 11)
#define XINPUT_MASK_A           (1u << 12)
#define XINPUT_MASK_B           (1u << 13)
#define XINPUT_MASK_X           (1u << 14)
#define XINPUT_MASK_Y           (1u << 15)

// 2. Sony PlayStation 4 DualShock 4 Report (64 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  report_id;    // 0x01
    uint8_t  lx;           // 0..255 (128 center)
    uint8_t  ly;           // 0..255 (128 center)
    uint8_t  rx;           // 0..255 (128 center)
    uint8_t  ry;           // 0..255 (128 center)
    uint8_t  dpad_buttons; // Low 4 bits: D-Pad Hat (0-7, 8=neutral), High 4 bits: Square, Cross, Circle, Triangle
    uint8_t  buttons_1;    // L1, R1, L2, R2, Share, Options, L3, R3
    uint8_t  buttons_2;    // PS Home, Touchpad Click, Counter (6 bits)
    uint8_t  l2_analog;    // 0..255
    uint8_t  r2_analog;    // 0..255
    uint16_t timestamp;
    uint8_t  battery;
    int16_t  gyro_x;
    int16_t  gyro_y;
    int16_t  gyro_z;
    int16_t  accel_x;
    int16_t  accel_y;
    int16_t  accel_z;
    uint8_t  reserved[39];
    uint32_t crc32;
} ds4_report_t;

// 3. Nintendo Switch Pro Controller Report
typedef struct __attribute__((packed)) {
    uint8_t  report_id;    // 0x30
    uint8_t  timer;
    uint8_t  battery_conn;
    uint8_t  btn_y : 1, btn_x : 1, btn_b : 1, btn_a : 1, btn_r_sr : 1, btn_r_sl : 1, btn_r : 1, btn_zr : 1;
    uint8_t  btn_minus : 1, btn_plus : 1, btn_r3 : 1, btn_l3 : 1, btn_home : 1, btn_capture : 1, btn_reserved1 : 2;
    uint8_t  btn_dpad_down : 1, btn_dpad_up : 1, btn_dpad_right : 1, btn_dpad_left : 1, btn_l_sr : 1, btn_l_sl : 1, btn_l : 1, btn_zl : 1;
    uint8_t  left_stick[3];  // 12-bit packed X/Y
    uint8_t  right_stick[3]; // 12-bit packed X/Y
    uint8_t  vibrator_data;
    int16_t  imu_data[6];
    uint8_t  padding[41];
} switch_pro_report_t;

// 4. Sony PlayStation 5 DualSense Report (64 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  report_id;    // 0x01
    uint8_t  lx;           // 0..255
    uint8_t  ly;           // 0..255
    uint8_t  rx;           // 0..255
    uint8_t  ry;           // 0..255
    uint8_t  l2_analog;    // 0..255
    uint8_t  r2_analog;    // 0..255
    uint8_t  seq_number;   // Packet counter
    uint8_t  dpad_buttons; // Low 4 bits: D-Pad Hat (0-7, 8=neutral), High 4 bits: Square, Cross, Circle, Triangle
    uint8_t  buttons_1;    // L1, R1, L2, R2, Create, Options, L3, R3
    uint8_t  buttons_2;    // PS Home, Touchpad Click, Mute Mic (bit 2)
    uint8_t  reserved[44];
    uint32_t crc32;
} dualsense_report_t;

// 5. Sony PlayStation 3 DualShock 3 Report (49 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  report_id;    // 0x01
    uint8_t  reserved1;
    uint8_t  btn_select : 1, btn_l3 : 1, btn_r3 : 1, btn_start : 1, btn_up : 1, btn_right : 1, btn_down : 1, btn_left : 1;
    uint8_t  btn_l2 : 1, btn_r2 : 1, btn_l1 : 1, btn_r1 : 1, btn_triangle : 1, btn_circle : 1, btn_cross : 1, btn_square : 1;
    uint8_t  btn_ps : 1, reserved_btn : 7;
    uint8_t  reserved2;
    uint8_t  lx;           // 0..255
    uint8_t  ly;           // 0..255
    uint8_t  rx;           // 0..255
    uint8_t  ry;           // 0..255
    uint8_t  reserved3[4];
    uint8_t  dpad_up_pressure;
    uint8_t  dpad_right_pressure;
    uint8_t  dpad_down_pressure;
    uint8_t  dpad_left_pressure;
    uint8_t  l2_pressure;
    uint8_t  r2_pressure;
    uint8_t  l1_pressure;
    uint8_t  r1_pressure;
    uint8_t  triangle_pressure;
    uint8_t  circle_pressure;
    uint8_t  cross_pressure;
    uint8_t  square_pressure;
    uint8_t  reserved4[24];
} ps3_report_t;

// Callback signature for host rumble events
typedef void (*host_rumble_callback_t)(uint8_t low_freq, uint8_t high_freq);

// Initializes TinyUSB subsystem for active profile
void usb_gamepad_init(controller_profile_t profile, host_rumble_callback_t rumble_cb);

// Polling task called in main cooperative loop
void usb_gamepad_task(void);

// Updates the outgoing gamepad state
void usb_gamepad_send_state(const gamepad_state_t *state);

// Returns true if USB device is mounted and configured by host
bool usb_gamepad_is_mounted(void);

#ifdef __cplusplus
}
#endif

#endif /* _USB_GAMEPAD_H_ */
