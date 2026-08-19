#include "usb_descriptors.h"
#include <string.h>

static controller_profile_t current_profile = PROFILE_XINPUT_XBOX360;

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

// 1. Xbox 360 / XInput (Microsoft)
static const tusb_desc_device_t desc_device_xinput = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0xFF, // Vendor Specific
    .bDeviceSubClass    = 0xFF,
    .bDeviceProtocol    = 0xFF,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x045E, // Microsoft Corp.
    .idProduct          = 0x028E, // Xbox 360 Controller
    .bcdDevice          = 0x0114,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// 2. Sony PlayStation 4 DualShock 4
static const tusb_desc_device_t desc_device_ds4 = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x054C, // Sony Interactive Entertainment Inc.
    .idProduct          = 0x05C4, // DualShock 4 Controller
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x00,
    .bNumConfigurations = 0x01
};

// 3. Nintendo Switch Pro Controller
static const tusb_desc_device_t desc_device_switch = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x057E, // Nintendo Co., Ltd.
    .idProduct          = 0x2009, // Switch Pro Controller
    .bcdDevice          = 0x0200,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// 4. Sony PlayStation 5 DualSense Controller
static const tusb_desc_device_t desc_device_dualsense = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x054C, // Sony Interactive Entertainment Inc.
    .idProduct          = 0x0CE6, // Wireless Controller (DualSense)
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// 5. Sony PlayStation 3 DualShock 3
static const tusb_desc_device_t desc_device_ps3 = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x054C, // Sony
    .idProduct          = 0x0268, // PlayStation 3 Controller
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x00,
    .bNumConfigurations = 0x01
};

//--------------------------------------------------------------------+
// HID Report Descriptors
//--------------------------------------------------------------------+

// Sony DS4 / DualSense HID Report Descriptor
static const uint8_t desc_hid_report_ds4[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (Eng Rot:Angular Pos)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,Null)
    0x65, 0x00,        //   Unit (None)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //   Usage (0x20)
    0x75, 0x06,        //   Report Size (6)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x33,        //   Usage (Rx - L2)
    0x09, 0x34,        //   Usage (Ry - R2)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x21,        //   Usage (0x21)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x36,        //   Report Count (54)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x85, 0x05,        //   Report ID (5 - Output / Rumble)
    0x09, 0x22,        //   Usage (0x22)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x1F,        //   Report Count (31)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0xC0               // End Collection
};

// Switch Pro HID Report Descriptor
static const uint8_t desc_hid_report_switch[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x10,        //   Usage Maximum (0x10)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,Null)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //   Usage (0x20)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x34,        //   Report Count (52)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x85, 0x01,        //   Report ID (1 - Output Rumble)
    0x09, 0x21,        //   Usage (0x21)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0xC0               // End Collection
};

//--------------------------------------------------------------------+
// Configuration Descriptors
//--------------------------------------------------------------------+

// 1. Xbox 360 / XInput Configuration Descriptor (Vendor Specific Class 0xFF)
// Total Length = 9 (Config) + 9 (Interface) + 17 (Xbox Header) + 7 (EP IN) + 7 (EP OUT) = 49 bytes (0x31)
static const uint8_t desc_configuration_xinput[] = {
    // Config Header (9 bytes)
    0x09, TUSB_DESC_CONFIGURATION, 0x31, 0x00, 0x01, 0x01, 0x00, 0x80, 0xFA, // 500mA
    // Interface 0 (9 bytes)
    0x09, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, 0xFF, 0x5D, 0x01, 0x00,
    // Unknown Xbox descriptor header (17 bytes)
    0x11, 0x21, 0x00, 0x01, 0x01, 0x25, 0x81, 0x14, 0x00, 0x00, 0x00, 0x00, 0x13, 0x02, 0x08, 0x00, 0x00,
    // Endpoint IN 1 (7 bytes)
    0x07, TUSB_DESC_ENDPOINT, EPNUM_XINPUT_IN, 0x03, 0x20, 0x00, 0x01, // Interrupt IN, 32 bytes, 1ms
    // Endpoint OUT 2 (7 bytes)
    0x07, TUSB_DESC_ENDPOINT, EPNUM_XINPUT_OUT, 0x03, 0x20, 0x00, 0x08  // Interrupt OUT, 32 bytes, 8ms
};

// 2. Sony PlayStation 4 DualShock 4 & DualSense Configuration Descriptor
static const uint8_t desc_configuration_ds4[] = {
    // Config Header (9 bytes)
    0x09, TUSB_DESC_CONFIGURATION, 0x29, 0x00, 0x01, 0x01, 0x00, 0x80, 0xFA, // 500mA
    // Interface 0: HID (9 bytes)
    0x09, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00,
    // HID Descriptor (9 bytes)
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, sizeof(desc_hid_report_ds4) & 0xFF, (sizeof(desc_hid_report_ds4) >> 8) & 0xFF,
    // Endpoint IN 1 (7 bytes)
    0x07, TUSB_DESC_ENDPOINT, EPNUM_HID_IN, 0x03, 0x40, 0x00, 0x01, // 64 bytes, 1ms
    // Endpoint OUT 1 (7 bytes)
    0x07, TUSB_DESC_ENDPOINT, EPNUM_HID_OUT, 0x03, 0x40, 0x00, 0x01  // 64 bytes, 1ms
};

// 3. Nintendo Switch Pro Controller Configuration Descriptor
static const uint8_t desc_configuration_switch[] = {
    // Config Header (9 bytes)
    0x09, TUSB_DESC_CONFIGURATION, 0x29, 0x00, 0x01, 0x01, 0x00, 0x80, 0xFA, // 500mA
    // Interface 0: HID (9 bytes)
    0x09, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00,
    // HID Descriptor (9 bytes)
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, sizeof(desc_hid_report_switch) & 0xFF, (sizeof(desc_hid_report_switch) >> 8) & 0xFF,
    // Endpoint IN 1 (7 bytes)
    0x07, TUSB_DESC_ENDPOINT, EPNUM_HID_IN, 0x03, 0x40, 0x00, 0x01, // 64 bytes, 1ms
    // Endpoint OUT 1 (7 bytes)
    0x07, TUSB_DESC_ENDPOINT, EPNUM_HID_OUT, 0x03, 0x40, 0x00, 0x01  // 64 bytes, 1ms
};

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
static char const* string_desc_arr_xinput[] = {
    (const char[]) { 0x09, 0x04 }, // 0: English (0x0409)
    "Microsoft Corp.",            // 1: Manufacturer
    "Controller",                 // 2: Product
    "1.00"                        // 3: Serial Number
};

static char const* string_desc_arr_sony[] = {
    (const char[]) { 0x09, 0x04 }, // 0: English
    "Sony Interactive Entertainment", // 1: Manufacturer
    "Wireless Controller",            // 2: Product
    "000000000001"                    // 3: Serial
};

static char const* string_desc_arr_switch[] = {
    (const char[]) { 0x09, 0x04 }, // 0: English
    "Nintendo Co., Ltd.",         // 1: Manufacturer
    "Pro Controller",             // 2: Product
    "000000000001"                // 3: Serial
};

static uint16_t _desc_str[64];

void usb_descriptors_set_profile(controller_profile_t profile) {
    current_profile = profile;
}

controller_profile_t usb_descriptors_get_profile(void) {
    return current_profile;
}

// Invoked when received GET DEVICE DESCRIPTOR
uint8_t const * tud_descriptor_device_cb(void) {
    switch (current_profile) {
        case PROFILE_XINPUT_XBOX360:
            return (uint8_t const *)&desc_device_xinput;
        case PROFILE_PS4_DUALSHOCK4:
            return (uint8_t const *)&desc_device_ds4;
        case PROFILE_SWITCH_PRO:
            return (uint8_t const *)&desc_device_switch;
        case PROFILE_PS5_DUALSENSE:
            return (uint8_t const *)&desc_device_dualsense;
        case PROFILE_PS3_DINPUT:
            return (uint8_t const *)&desc_device_ps3;
        default:
            return (uint8_t const *)&desc_device_xinput;
    }
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    switch (current_profile) {
        case PROFILE_XINPUT_XBOX360:
            return desc_configuration_xinput;
        case PROFILE_SWITCH_PRO:
            return desc_configuration_switch;
        case PROFILE_PS4_DUALSHOCK4:
        case PROFILE_PS5_DUALSENSE:
        case PROFILE_PS3_DINPUT:
        default:
            return desc_configuration_ds4;
    }
}

// Invoked when received GET HID REPORT DESCRIPTOR
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    if (current_profile == PROFILE_SWITCH_PRO) {
        return desc_hid_report_switch;
    }
    return desc_hid_report_ds4;
}

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    uint8_t chr_count;

    char const** str_arr;
    if (current_profile == PROFILE_XINPUT_XBOX360) {
        str_arr = string_desc_arr_xinput;
    } else if (current_profile == PROFILE_SWITCH_PRO) {
        str_arr = string_desc_arr_switch;
    } else {
        str_arr = string_desc_arr_sony;
    }

    if (index == 0) {
        memcpy(&_desc_str[1], str_arr[0], 2);
        chr_count = 1;
    } else {
        if (index > 3) return NULL;
        const char* str = str_arr[index];
        chr_count = (uint8_t)strlen(str);
        if (chr_count > 63) chr_count = 63;

        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}
