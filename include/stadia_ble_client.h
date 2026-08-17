#ifndef _STADIA_BLE_CLIENT_H_
#define _STADIA_BLE_CLIENT_H_

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// BLE Connection States
typedef enum {
    BLE_STATE_OFF = 0,
    BLE_STATE_SCANNING,
    BLE_STATE_CONNECTING,
    BLE_STATE_PAIRING_AUTHENTICATING,
    BLE_STATE_DISCOVERING_SERVICES,
    BLE_STATE_SUBSCRIBING_HOGP,
    BLE_STATE_CONNECTED_STREAMING,
    BLE_STATE_DISCONNECTED
} ble_connection_state_t;

// Callback signature when new normalized gamepad state arrives
typedef void (*stadia_state_callback_t)(const gamepad_state_t *state);

// Initializes the BTstack BLE stack and CYW43 chip
void stadia_ble_init(stadia_state_callback_t callback);

// Periodic polling task for BTstack / CYW43
void stadia_ble_task(void);

// Returns current BLE connection state
ble_connection_state_t stadia_ble_get_state(void);

// Returns true if Stadia Controller is actively connected and streaming
bool stadia_ble_is_connected(void);

// Send rumble / force-feedback command to the Stadia Controller
bool stadia_ble_send_rumble(uint8_t low_freq, uint8_t high_freq);

// Trigger re-scan / unpair / clear bonding
void stadia_ble_clear_bonding_and_rescan(void);

#ifdef __cplusplus
}
#endif

#endif /* _STADIA_BLE_CLIENT_H_ */
