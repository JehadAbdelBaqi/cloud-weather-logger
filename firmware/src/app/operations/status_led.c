#include "status_led.h"

#include <stdint.h>

#include "app_state.h"
#include "config.h"
#include "led_driver.h"
#include "time_interval.h"

/**
 * @brief Blinks the on-board LED to show the state: slowly when the state is ok, quickly when not.
 *
 * Call on every pass: the LED is toggled once its blink time has passed.
 *
 * @param state The program's running state
 */
void handle_led(app_state_t *state)
{
    static uint32_t last_toggle_ms = 0;
    uint32_t blink_ms = (state->current_state == STATE_OK) ? LED_CONNECTED_BLINK_MS : LED_CONNECTING_BLINK_MS;

    if (!time_interval_passed(state->now_ms, last_toggle_ms, blink_ms)) {
        return;
    }
    last_toggle_ms = state->now_ms;
    led_toggle();
}
