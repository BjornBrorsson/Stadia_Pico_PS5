#include "status_led.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

static status_led_mode_t current_mode = LED_MODE_OFF;
static uint32_t last_toggle_time = 0;
static bool led_state = false;
static uint32_t activity_pulse_end = 0;

void status_led_init(void) {
    current_mode = LED_MODE_OFF;
    led_state = false;
    last_toggle_time = to_ms_since_boot(get_absolute_time());
    activity_pulse_end = 0;
}

void status_led_set_mode(status_led_mode_t mode) {
    current_mode = mode;
}

void status_led_trigger_activity(void) {
    if (current_mode == LED_MODE_CONNECTED_READY) {
        activity_pulse_end = to_ms_since_boot(get_absolute_time()) + 30; // 30ms blip
    }
}

void status_led_task(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (current_mode) {
        case LED_MODE_OFF:
            if (led_state) {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                led_state = false;
            }
            break;

        case LED_MODE_SCANNING:
            // Fast blink: 100ms on, 100ms off
            if (now - last_toggle_time >= 100) {
                last_toggle_time = now;
                led_state = !led_state;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state ? 1 : 0);
            }
            break;

        case LED_MODE_CONNECTING:
            // Medium blink: 250ms on, 250ms off
            if (now - last_toggle_time >= 250) {
                last_toggle_time = now;
                led_state = !led_state;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state ? 1 : 0);
            }
            break;

        case LED_MODE_CONNECTED_READY:
            // Solid ON with quick dip on activity
            if (now < activity_pulse_end) {
                // Momentary dip
                if (led_state) {
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                    led_state = false;
                }
            } else {
                if (!led_state) {
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                    led_state = true;
                }
            }
            break;

        case LED_MODE_PAIRING_RESET:
            // Rapid strobe (50ms)
            if (now - last_toggle_time >= 50) {
                last_toggle_time = now;
                led_state = !led_state;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state ? 1 : 0);
            }
            break;

        default:
            break;
    }
}
