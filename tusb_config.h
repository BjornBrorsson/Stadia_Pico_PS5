#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+
#define CFG_TUSB_MCU                OPT_MCU_RP2040
#define CFG_TUSB_OS                 OPT_OS_NONE

// Device mode enabled
#define CFG_TUD_ENABLED             1

// USB Port 0 Full Speed
#define CFG_TUSB_RHPORT0_MODE       (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

// Endpoint 0 max packet size
#define CFG_TUD_ENDPOINT0_SIZE      64

//--------------------------------------------------------------------+
// Class Configuration
//--------------------------------------------------------------------+
// Enable HID driver (DualShock 4, Nintendo Switch Pro, DualSense, PS3)
#define CFG_TUD_HID                 1

// Enable Vendor driver (XInput Xbox 360 emulation)
#define CFG_TUD_VENDOR              1

// HID buffer sizes
#define CFG_TUD_HID_EP_BUFSIZE      64

// Vendor buffer sizes (for XInput In/Out packets)
#define CFG_TUD_VENDOR_RX_BUFSIZE   64
#define CFG_TUD_VENDOR_TX_BUFSIZE   64

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
