#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Available Output Controller Profiles
typedef enum {
    PROFILE_XINPUT_XBOX360 = 0, // Microsoft Xbox 360 (Default & Recommended for Brook Wingman P5)
    PROFILE_PS4_DUALSHOCK4  = 1, // Sony DualShock 4 (Direct / Wingman P5)
    PROFILE_SWITCH_PRO      = 2, // Nintendo Switch Pro Controller (Wingman P5 / Switch)
    PROFILE_PS5_DUALSENSE   = 3, // Sony DualSense PS5 USB Emulation
    PROFILE_PS3_DINPUT      = 4, // Sony DualShock 3 / Generic DInput
    PROFILE_COUNT           = 5
} controller_profile_t;

// Normalized Button Bitmask Definitions
#define BTN_MASK_CROSS_A        (1u << 0)  // Cross (PS) / A (Xbox) / B (Switch)
#define BTN_MASK_CIRCLE_B       (1u << 1)  // Circle (PS) / B (Xbox) / A (Switch)
#define BTN_MASK_SQUARE_X       (1u << 2)  // Square (PS) / X (Xbox) / Y (Switch)
#define BTN_MASK_TRIANGLE_Y     (1u << 3)  // Triangle (PS) / Y (Xbox) / X (Switch)
#define BTN_MASK_L1             (1u << 4)  // Left Bumper / L1 / L
#define BTN_MASK_R1             (1u << 5)  // Right Bumper / R1 / R
#define BTN_MASK_L2_DIGITAL     (1u << 6)  // L2 click threshold
#define BTN_MASK_R2_DIGITAL     (1u << 7)  // R2 click threshold
#define BTN_MASK_SELECT_SHARE   (1u << 8)  // Options (...) / Share / Back / Minus
#define BTN_MASK_START_OPTIONS  (1u << 9)  // Menu (☰) / Options / Start / Plus
#define BTN_MASK_L3             (1u << 10) // Left Stick Click
#define BTN_MASK_R3             (1u << 11) // Right Stick Click
#define BTN_MASK_HOME_GUIDE     (1u << 12) // Stadia button / PS Home / Xbox Guide
#define BTN_MASK_TOUCHPAD_ASSIST(1u << 13) // Assistant button (●) / Touchpad Click
#define BTN_MASK_CAPTURE_MUTE   (1u << 14) // Capture button ([ ]) / Mic Mute

// D-Pad Directional Bitmask
#define DPAD_MASK_UP            (1u << 0)
#define DPAD_MASK_DOWN          (1u << 1)
#define DPAD_MASK_LEFT          (1u << 2)
#define DPAD_MASK_RIGHT         (1u << 3)

// Universal Normalized Gamepad State Structure
typedef struct {
    uint16_t buttons;      // Bitmask of BTN_MASK_*
    uint8_t  dpad;         // Bitmask of DPAD_MASK_*
    
    // Analog Joysticks: -32768 (Left/Up) to +32767 (Right/Down), 0 = Centered
    int16_t  stick_lx;
    int16_t  stick_ly;
    int16_t  stick_rx;
    int16_t  stick_ry;
    
    // Analog Triggers: 0 (Released) to 255 (Fully pressed)
    uint8_t  trigger_l;
    uint8_t  trigger_r;
    
    // Auxiliary data
    uint8_t  battery_level;
    bool     connected;
} gamepad_state_t;

// Rumble/Haptic state sent from Host to Controller
typedef struct {
    uint8_t  low_frequency_rumble;  // Large motor (0-255)
    uint8_t  high_frequency_rumble; // Small motor (0-255)
} rumble_state_t;

// Flash Memory Persistence Config
#define FLASH_CONFIG_OFFSET (1024 * 1024) // 1MB offset into Flash
#define FLASH_CONFIG_MAGIC  0x53544431    // 'STD1'

typedef struct {
    uint32_t magic;
    uint8_t  active_profile;
    uint8_t  reserved[3];
} persistent_config_t;

#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_H_ */
