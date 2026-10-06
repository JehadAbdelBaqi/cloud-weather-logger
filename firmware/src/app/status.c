#include "status.h"

#include <stdint.h>

#include "stm32f4xx_hal.h"

#include "config.h"
#include "handshake.h"
#include "led_driver.h"
#include "time_interval.h"

void status_poll(void)
{
    static uint32_t last_toggle_ms = 0;
    uint32_t blink_ms = handshake_connected() ? LED_CONNECTED_BLINK_MS : LED_CONNECTING_BLINK_MS;

    if (!time_interval_passed(HAL_GetTick(), last_toggle_ms, blink_ms)) {
        return;
    }

    last_toggle_ms = HAL_GetTick();
    led_toggle();
}
