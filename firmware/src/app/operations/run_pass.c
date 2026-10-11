#include "run_pass.h"

#include <stdint.h>

#include "app_state.h"
#include "config.h"
#include "poll_uart.h"
#include "reading.h"
#include "reading_store.h"
#include "sensor_timer.h"
#include "states.h"
#include "status_led.h"

/**
 * @brief Runs one pass of the main loop: the things done whatever the state is, then the state.
 *
 * Takes a reading if one is due and stores it, polls the UART, runs the current state, then blinks
 * the LED. Reads no clock: it is handed the time.
 *
 * @param state   The program's running state: its time is set to the time now
 * @param time_ms The time now, as a reading of a millisecond counter such as HAL_GetTick()
 */
void run_pass(app_state_t *state, uint32_t time_ms)
{
    reading_t reading;

    state->now_ms = time_ms;
    if (sensor_poll(&state->sensor_timer, state->now_ms, &reading)) {
        reading_store_put(&reading, READING_TIME_START + state->now_ms / 1000);
    }
    poll_uart(state);
    state_setter(state);
    handle_led(state);
}
