#include "stm32f4xx_hal.h"

#include "clock.h"

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

    while (1) {
    }
}
