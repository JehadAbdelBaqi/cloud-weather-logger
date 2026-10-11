#include "stm32f4xx_hal.h"

#include "app_state.h"
#include "board.h"
#include "config.h"
#include "debug.h"
#include "run_pass.h"
#include "sensor.h"

// What lasts from one pass of the main loop to the next. The state starts as no response, so a
// handshake is the first thing sent, and the read interval starts as the one in the config
static app_state_t app_state = {
    .current_state = STATE_NO_RESPONSE,
    .sensor_timer = { .read_interval_ms = SENSOR_READ_INTERVAL_MS },
};

/**
 * @brief Stops the program after a setup step has failed. Never returns.
 */
static void error_halt(void)
{
    __disable_irq();
    while (1) {
    }
}

/**
 * @brief Entry point, called by the startup code after reset.
 */
int main(void)
{
    if (!board_init()) {
        error_halt();
    }

    debug_print("weather logger started\r\n");
    sensor_init();

    while (1) {
        run_pass(&app_state, HAL_GetTick());
    }
}
