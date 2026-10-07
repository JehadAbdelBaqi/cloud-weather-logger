#include "stm32f4xx_hal.h"

#include "board.h"
#include "debug.h"
#include "polling.h"
#include "protocol.h"
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

    while (1) {
        router_poll();
        polling_send_when_due(PROTOCOL_LETTER_HANDSHAKE, PROTOCOL_HANDSHAKE_REQUEST);
        status_poll();
        sensor_poll();
    }
}
