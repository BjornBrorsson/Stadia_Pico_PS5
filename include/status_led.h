#ifndef _STATUS_LED_H_
#define _STATUS_LED_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_SCANNING,            // Fast continuous blink (100ms on, 100ms off)
    LED_MODE_CONNECTING,          // Medium blink (250ms on, 250ms off)
    LED_MODE_CONNECTED_READY,     // Solid ON
    LED_MODE_ACTIVITY_PULSE,      // Solid ON with quick dip on input
    LED_MODE_PAIRING_RESET,       // 3 rapid flashes
    LED_MODE_PROFILE_INDICATOR    // Blinks active profile ID
} status_led_mode_t;

// Initialize LED system
void status_led_init(void);

// Set current LED pattern
void status_led_set_mode(status_led_mode_t mode);

// Trigger a momentary activity blip
void status_led_trigger_activity(void);

// Periodic update task
void status_led_task(void);

#ifdef __cplusplus
}
#endif

#endif /* _STATUS_LED_H_ */
