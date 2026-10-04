#include <stddef.h>

#include "stm32f4xx_hal.h"

#include "clock.h"
#include "debug.h"
#include "led.h"
#include "link.h"

// How often the test line is sent, in milliseconds
#define TEST_LINE_INTERVAL_MS 1000

/**
 * @brief SysTick interrupt, fires every 1 ms. Advances the HAL's millisecond counter.
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

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
 * @brief Sends the test line to the bridge and toggles the LED, once the interval has passed.
 *
 * Does not wait: returns straight away when it is not time yet.
 */
static void send_test_line_when_due(void)
{
    static uint32_t last_sent_ms = 0;

    if (HAL_GetTick() - last_sent_ms < TEST_LINE_INTERVAL_MS) {
        return;
    }

    last_sent_ms = HAL_GetTick();
    led_toggle();
    link_send_line("T,test line from the nucleo");
}

/**
 * @brief Writes a line that has arrived from the bridge to the debug output, if one is complete.
 */
static void show_received_line(void)
{
    const char *line = link_read_line();
    if (line == NULL) {
        return;
    }

    debug_print("down: ");
    debug_print(line);
    debug_print("\r\n");
}

/**
 * @brief Entry point, called by the startup code after reset.
 */
int main(void)
{
    HAL_Init();
    if (!clock_init()) {
        error_halt();
    }
    led_init();
    if (!debug_init()) {
        error_halt();
    }
    if (!link_init()) {
        error_halt();
    }

    debug_print("weather logger started\r\n");

    while (1) {
        send_test_line_when_due();
        show_received_line();
    }
}
