#include "stm32f4xx_hal.h"

#include "clock.h"
#include "led.h"
#include "link.h"

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
 * @brief Entry point, called by the startup code after reset.
 */
int main(void)
{
    HAL_Init();
    if (!clock_init()) {
        error_halt();
    }
    led_init();
    if (!link_init()) {
        error_halt();
    }

    while (1) {
        led_toggle();
        link_send_line("T,test line from the nucleo");
        HAL_Delay(1000);
    }
}
