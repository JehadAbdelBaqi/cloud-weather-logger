#include "stm32f4xx_hal.h"

#include "board.h"
#include "debug.h"
#include "handshake.h"
#include "router.h"
#include "sensor.h"
#include "status.h"

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
    handshake_init();

    while (1) {
        router_poll();
        handshake_poll();
        status_poll();
        sensor_poll();
    }
}
