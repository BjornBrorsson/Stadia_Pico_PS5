#ifndef _USB_DESCRIPTORS_H_
#define _USB_DESCRIPTORS_H_

#include <stdint.h>
#include "tusb.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Endpoint Numbers
#define EPNUM_HID_IN        0x81
#define EPNUM_HID_OUT       0x01

#define EPNUM_XINPUT_IN     0x81
#define EPNUM_XINPUT_OUT    0x02

// Set the active profile before USB stack initialization
void usb_descriptors_set_profile(controller_profile_t profile);
controller_profile_t usb_descriptors_get_profile(void);

// Raw USB Descriptors accessors
const tusb_desc_device_t* usb_desc_get_device(void);
const uint8_t* usb_desc_get_configuration(void);
const uint8_t* usb_desc_get_hid_report(void);
uint16_t usb_desc_get_hid_report_size(void);

#ifdef __cplusplus
}
#endif

#endif /* _USB_DESCRIPTORS_H_ */
