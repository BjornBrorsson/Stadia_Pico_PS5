#include "stadia_ble_client.h"
#include "input_mapper.h"
#include "status_led.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include "ble/gatt-service/hids_host.h"
#include "ble/le_device_db.h"
#include <string.h>
#include <stdio.h>

// Standard BLE UUIDs
#define ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE          0x1812

// Stadia Controller BLE Identifiers
#define STADIA_BLE_NAME_PREFIX "Stadia"
#define STADIA_INPUT_REPORT_ID 0x03
#define STADIA_OUTPUT_REPORT_ID 0x05

// Module state
static ble_connection_state_t ble_state = BLE_STATE_OFF;
static stadia_state_callback_t state_callback = NULL;

static bd_addr_t stadia_addr;
static bd_addr_type_t stadia_addr_type;
static hci_con_handle_t connection_handle = HCI_CON_HANDLE_INVALID;
static uint16_t hids_cid = 0;

static uint8_t hid_descriptor_storage[600];

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

//--------------------------------------------------------------------+
// GAP Advertisement & Name Parsing Helpers
//--------------------------------------------------------------------+

static bool advertisement_matches_stadia(const uint8_t *adv_data, uint8_t adv_len) {
    ad_context_t context;
    for (ad_iterator_init(&context, adv_len, adv_data); ad_iterator_has_more(&context); ad_iterator_next(&context)) {
        uint8_t data_type    = ad_iterator_get_data_type(&context);
        uint8_t size         = ad_iterator_get_data_len(&context);
        const uint8_t *data  = ad_iterator_get_data(&context);

        // Check Complete / Shortened Local Name (Types 0x09 and 0x08)
        if (data_type == BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME || 
            data_type == BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME) {
            if (size >= 6 && memcmp(data, "Stadia", 6) == 0) {
                return true;
            }
        }

        // Check 16-bit Service UUIDs (Type 0x02 or 0x03)
        if (data_type == BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS ||
            data_type == BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS) {
            for (int i = 0; i < size; i += 2) {
                uint16_t uuid = little_endian_read_16(data, i);
                if (uuid == ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE) {
                    return true;
                }
            }
        }

        // Check Appearance (0x03C4 = Gamepad)
        if (data_type == BLUETOOTH_DATA_TYPE_APPEARANCE && size == 2) {
            uint16_t appearance = little_endian_read_16(data, 0);
            if (appearance == 0x03C4 || appearance == 0x03C0) {
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------+
// HIDS Host Events
//--------------------------------------------------------------------+

static void handle_hids_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) packet_type;
    (void) channel;
    (void) size;

    if (hci_event_packet_get_type(packet) != HCI_EVENT_GATTSERVICE_META) return;

    switch (hci_event_gattservice_meta_get_subevent_code(packet)) {
        case GATTSERVICE_SUBEVENT_HID_SERVICE_CONNECTED: {
            uint8_t status = gattservice_subevent_hid_service_connected_get_status(packet);
            if (status == ERROR_CODE_SUCCESS) {
                ble_state = BLE_STATE_CONNECTED_STREAMING;
                status_led_set_mode(LED_MODE_CONNECTED_READY);
            } else {
                gap_disconnect(connection_handle);
            }
            break;
        }

        case GATTSERVICE_SUBEVENT_HID_SERVICE_DISCONNECTED: {
            hids_cid = 0;
            ble_state = BLE_STATE_SCANNING;
            status_led_set_mode(LED_MODE_SCANNING);
            gap_start_scan();
            break;
        }

        case GATTSERVICE_SUBEVENT_HID_REPORT: {
            const uint8_t *report = gattservice_subevent_hid_report_get_report(packet);
            uint16_t report_len = gattservice_subevent_hid_report_get_report_len(packet);
            if (report && report_len >= 9 && state_callback) {
                gamepad_state_t current_state;
                if (input_mapper_parse_stadia_report(report, report_len, &current_state)) {
                    state_callback(&current_state);
                }
            }
            break;
        }

        default:
            break;
    }
}

//--------------------------------------------------------------------+
// HCI & Security Manager Event Callbacks
//--------------------------------------------------------------------+

static void sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;

    if (packet_type != HCI_EVENT_PACKET) return;

    bool connect_to_service = false;

    switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;

        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            sm_numeric_comparison_confirm(sm_event_numeric_comparison_request_get_handle(packet));
            break;

        case SM_EVENT_PAIRING_COMPLETE:
            if (sm_event_pairing_complete_get_status(packet) == ERROR_CODE_SUCCESS) {
                connect_to_service = true;
            }
            break;

        case SM_EVENT_REENCRYPTION_COMPLETE:
            if (sm_event_reencryption_complete_get_status(packet) == ERROR_CODE_SUCCESS) {
                connect_to_service = true;
            }
            break;

        default:
            break;
    }

    if (connect_to_service && connection_handle != HCI_CON_HANDLE_INVALID) {
        ble_state = BLE_STATE_DISCOVERING_SERVICES;
        hids_host_connect(connection_handle, handle_hids_client_event, HID_PROTOCOL_MODE_REPORT, &hids_cid);
    }
}

static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;

    if (packet_type != HCI_EVENT_PACKET) return;

    uint8_t event = hci_event_packet_get_type(packet);

    switch (event) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                ble_state = BLE_STATE_SCANNING;
                status_led_set_mode(LED_MODE_SCANNING);
                gap_set_scan_parameters(1, 0x0030, 0x0030); // Active scanning
                gap_start_scan();
            }
            break;

        case GAP_EVENT_ADVERTISING_REPORT: {
            if (ble_state != BLE_STATE_SCANNING) break;

            uint8_t adv_len = gap_event_advertising_report_get_data_length(packet);
            const uint8_t *adv_data = gap_event_advertising_report_get_data(packet);

            if (advertisement_matches_stadia(adv_data, adv_len)) {
                gap_stop_scan();
                gap_event_advertising_report_get_address(packet, stadia_addr);
                stadia_addr_type = (bd_addr_type_t)gap_event_advertising_report_get_address_type(packet);

                ble_state = BLE_STATE_CONNECTING;
                status_led_set_mode(LED_MODE_CONNECTING);

                // Connect with 7.5ms - 15ms connection interval for low latency
                gap_connect(stadia_addr, stadia_addr_type);
            }
            break;
        }

        case HCI_EVENT_META_GAP: {
            if (hci_event_gap_meta_get_subevent_code(packet) == GAP_SUBEVENT_LE_CONNECTION_COMPLETE) {
                uint8_t status = gap_subevent_le_connection_complete_get_status(packet);
                if (status == ERROR_CODE_SUCCESS) {
                    connection_handle = gap_subevent_le_connection_complete_get_connection_handle(packet);
                    ble_state = BLE_STATE_PAIRING_AUTHENTICATING;

                    // Request fast 7.5ms-15ms interval
                    gap_request_connection_parameter_update(connection_handle, 6, 12, 0, 300);

                    // Trigger SM pairing / encryption
                    sm_request_pairing(connection_handle);
                } else {
                    ble_state = BLE_STATE_SCANNING;
                    status_led_set_mode(LED_MODE_SCANNING);
                    gap_start_scan();
                }
            }
            break;
        }

        case HCI_EVENT_LE_META: {
            uint8_t subevent = hci_event_le_meta_get_subevent_code(packet);
            if (subevent == HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                uint8_t status = hci_subevent_le_connection_complete_get_status(packet);
                if (status == ERROR_CODE_SUCCESS && connection_handle == HCI_CON_HANDLE_INVALID) {
                    connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                    ble_state = BLE_STATE_PAIRING_AUTHENTICATING;

                    gap_request_connection_parameter_update(connection_handle, 6, 12, 0, 300);
                    sm_request_pairing(connection_handle);
                }
            }
            break;
        }

        case HCI_EVENT_DISCONNECTION_COMPLETE: {
            connection_handle = HCI_CON_HANDLE_INVALID;
            hids_cid = 0;

            ble_state = BLE_STATE_SCANNING;
            status_led_set_mode(LED_MODE_SCANNING);
            gap_start_scan();
            break;
        }

        default:
            break;
    }
}

//--------------------------------------------------------------------+
// Public API
//--------------------------------------------------------------------+

void stadia_ble_init(stadia_state_callback_t callback) {
    state_callback = callback;
    ble_state = BLE_STATE_OFF;
    connection_handle = HCI_CON_HANDLE_INVALID;
    hids_cid = 0;

    l2cap_init();
    gatt_client_init();

    // Initialize HIDS Host service client
    hids_host_init(hid_descriptor_storage, sizeof(hid_descriptor_storage));

    // Register HCI and Security Manager event handlers
    hci_event_callback_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    sm_event_callback_registration.callback = &sm_packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    // Setup Security Manager: Just Works + Bonding
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_BONDING | SM_AUTHREQ_SECURE_CONNECTION);

    // Turn on BTstack HCI
    hci_power_control(HCI_POWER_ON);
}

void stadia_ble_task(void) {
    // CYW43 background polling handled by pico_cyw43_arch
}

ble_connection_state_t stadia_ble_get_state(void) {
    return ble_state;
}

bool stadia_ble_is_connected(void) {
    return (ble_state == BLE_STATE_CONNECTED_STREAMING) && (connection_handle != HCI_CON_HANDLE_INVALID);
}

bool stadia_ble_send_rumble(uint8_t low_freq, uint8_t high_freq) {
    if (!stadia_ble_is_connected() || hids_cid == 0) return false;

    // Stadia Rumble Protocol: Report ID 0x05 (5 bytes)
    // [0x05, low_frequency_low, low_frequency_high, high_frequency_low, high_frequency_high]
    uint8_t rumble_packet[5];
    rumble_packet[0] = STADIA_OUTPUT_REPORT_ID;
    rumble_packet[1] = low_freq;
    rumble_packet[2] = (low_freq > 0) ? 0x80 : 0x00;
    rumble_packet[3] = high_freq;
    rumble_packet[4] = (high_freq > 0) ? 0x80 : 0x00;

    hids_host_send_write_report(hids_cid, STADIA_OUTPUT_REPORT_ID, HID_REPORT_TYPE_OUTPUT, rumble_packet, sizeof(rumble_packet));
    return true;
}

void stadia_ble_clear_bonding_and_rescan(void) {
    if (connection_handle != HCI_CON_HANDLE_INVALID) {
        gap_disconnect(connection_handle);
    }
    int count = le_device_db_count();
    for (int i = 0; i < count; i++) {
        le_device_db_remove(0);
    }
    connection_handle = HCI_CON_HANDLE_INVALID;
    hids_cid = 0;
    ble_state = BLE_STATE_SCANNING;
    status_led_set_mode(LED_MODE_PAIRING_RESET);
    gap_start_scan();
}
