#include "stadia_ble_client.h"
#include "input_mapper.h"
#include "status_led.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include <string.h>
#include <stdio.h>

// Standard BLE UUIDs
#define ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE          0x1812
#define ORG_BLUETOOTH_CHARACTERISTIC_REPORT                   0x2A4D
#define ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP               0x2A4B
#define ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT 0x2A22
#define ORG_BLUETOOTH_DESCRIPTOR_REPORT_REFERENCE             0x2908
#define ORG_BLUETOOTH_DESCRIPTOR_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION 0x2902

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

static gatt_client_service_t hid_service;
static gatt_client_characteristic_t input_report_characteristic;
static gatt_client_characteristic_t output_report_characteristic;
static gatt_client_notification_t notification_listener;

static bool found_input_report = false;
static bool found_output_report = false;
static bool hogp_subscribed = false;

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
// GATT Discovery & HOGP Handlers
//--------------------------------------------------------------------+

static void handle_gatt_notification(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) packet_type;
    (void) channel;

    if (hci_event_packet_get_type(packet) != GATT_EVENT_NOTIFICATION) return;

    uint16_t value_length = gatt_event_notification_get_value_length(packet);
    const uint8_t *value  = gatt_event_notification_get_value(packet);

    if (value && value_length >= 10 && state_callback) {
        gamepad_state_t current_state;
        if (input_mapper_parse_stadia_report(value, value_length, &current_state)) {
            state_callback(&current_state);
        }
    }
}

static void gatt_characteristic_discovery_callback(uint8_t packet_type, uint16_t channel, uint8_t *packet, void *arg) {
    (void) packet_type;
    (void) channel;
    (void) arg;

    uint8_t event = hci_event_packet_get_type(packet);

    switch (event) {
        case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT: {
            gatt_client_characteristic_t characteristic;
            gatt_event_characteristic_query_result_get_characteristic(packet, &characteristic);

            if (characteristic.uuid16 == ORG_BLUETOOTH_CHARACTERISTIC_REPORT) {
                if (characteristic.properties & (ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ)) {
                    input_report_characteristic = characteristic;
                    found_input_report = true;
                }
                if (characteristic.properties & (ATT_PROPERTY_WRITE | ATT_PROPERTY_WRITE_WITHOUT_RESPONSE)) {
                    output_report_characteristic = characteristic;
                    found_output_report = true;
                }
            }
            break;
        }

        case GATT_EVENT_QUERY_COMPLETE: {
            if (found_input_report) {
                ble_state = BLE_STATE_SUBSCRIBING_HOGP;
                // Subscribe to notifications
                gatt_client_listen_for_characteristic_value_updates(
                    &notification_listener,
                    handle_gatt_notification,
                    connection_handle,
                    &input_report_characteristic
                );

                gatt_client_write_client_characteristic_configuration(
                    NULL,
                    connection_handle,
                    &input_report_characteristic,
                    GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION
                );

                hogp_subscribed = true;
                ble_state = BLE_STATE_CONNECTED_STREAMING;
                status_led_set_mode(LED_MODE_CONNECTED_READY);
            }
            break;
        }
        default:
            break;
    }
}

static void gatt_service_discovery_callback(uint8_t packet_type, uint16_t channel, uint8_t *packet, void *arg) {
    (void) packet_type;
    (void) channel;
    (void) arg;

    uint8_t event = hci_event_packet_get_type(packet);

    switch (event) {
        case GATT_EVENT_SERVICE_QUERY_RESULT: {
            gatt_client_service_t service;
            gatt_event_service_query_result_get_service(packet, &service);
            if (service.uuid16 == ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE) {
                hid_service = service;
            }
            break;
        }

        case GATT_EVENT_QUERY_COMPLETE: {
            if (hid_service.start_group_handle != 0) {
                ble_state = BLE_STATE_DISCOVERING_SERVICES;
                gatt_client_discover_characteristics_for_service(
                    gatt_characteristic_discovery_callback,
                    connection_handle,
                    &hid_service,
                    NULL
                );
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
    (void) packet_type;
    (void) channel;
    (void) size;

    uint8_t event = hci_event_packet_get_type(packet);

    switch (event) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;

        case SM_EVENT_PAIRING_COMPLETE:
            if (sm_event_pairing_complete_get_status(packet) == ERROR_CODE_SUCCESS) {
                // Request fast connection parameters (7.5ms to 15ms interval)
                gap_request_connection_parameter_update(connection_handle, 6, 12, 0, 300);

                // Start GATT Service Discovery
                gatt_client_discover_primary_services_by_uuid16(
                    gatt_service_discovery_callback,
                    connection_handle,
                    ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE,
                    NULL
                );
            }
            break;
        default:
            break;
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

        case HCI_EVENT_LE_META: {
            uint8_t subevent = hci_event_le_meta_get_subevent_code(packet);
            if (subevent == HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                uint8_t status = hci_subevent_le_connection_complete_get_status(packet);
                if (status == ERROR_CODE_SUCCESS) {
                    connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                    ble_state = BLE_STATE_PAIRING_AUTHENTICATING;

                    // Trigger SM pairing / encryption
                    sm_request_pairing(connection_handle);
                } else {
                    // Re-enter scanning mode
                    ble_state = BLE_STATE_SCANNING;
                    status_led_set_mode(LED_MODE_SCANNING);
                    gap_start_scan();
                }
            }
            break;
        }

        case HCI_EVENT_DISCONNECTION_COMPLETE: {
            connection_handle = HCI_CON_HANDLE_INVALID;
            found_input_report = false;
            found_output_report = false;
            hogp_subscribed = false;
            memset(&hid_service, 0, sizeof(hid_service));

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
    if (!stadia_ble_is_connected() || !found_output_report) return false;

    // Stadia Rumble Protocol: Report ID 0x05 (5 bytes)
    // [0x05, low_frequency_low, low_frequency_high, high_frequency_low, high_frequency_high]
    uint8_t rumble_packet[5];
    rumble_packet[0] = STADIA_OUTPUT_REPORT_ID;
    rumble_packet[1] = low_freq;
    rumble_packet[2] = (low_freq > 0) ? 0x80 : 0x00;
    rumble_packet[3] = high_freq;
    rumble_packet[4] = (high_freq > 0) ? 0x80 : 0x00;

    gatt_client_write_value_of_characteristic_without_response(
        connection_handle,
        output_report_characteristic.value_handle,
        sizeof(rumble_packet),
        rumble_packet
    );

    return true;
}

void stadia_ble_clear_bonding_and_rescan(void) {
    if (connection_handle != HCI_CON_HANDLE_INVALID) {
        gap_disconnect(connection_handle);
    }
    gap_delete_all_pairings();
    ble_state = BLE_STATE_SCANNING;
    status_led_set_mode(LED_MODE_PAIRING_RESET);
    gap_start_scan();
}
