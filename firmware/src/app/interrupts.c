#include "stm32f4xx_hal.h"

// The interrupt handlers. Each replaces the empty handler of the same name in the startup code.

/**
 * @brief SysTick interrupt, fires every 1 ms. Advances the HAL's millisecond counter.
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
