#include "stm32f4xx_hal.h"

#include "board_config.h"
#include "link.h"

// The interrupt handlers. Each replaces the empty handler of the same name in the startup code.

/**
 * @brief SysTick interrupt, fires every 1 ms. Advances the HAL's millisecond counter.
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
 * @brief Interrupt of the link's UART, fires each time a byte arrives from the bridge. Hands the
 * byte to the link, which keeps it until the main loop reads it.
 */
void LINK_INTERRUPT_HANDLER(void)
{
    link_byte_arrived();
}
