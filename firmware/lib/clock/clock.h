#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>

/**
 * @brief Sets the system clock: the internal 16 MHz oscillator (HSI), no PLL, no bus dividers.
 *
 * This is the state the chip is already in after reset; it is written out so the rest of the
 * firmware does not depend on the reset values. Call once, straight after HAL_Init().
 *
 * @return true if the clock was set, false if the HAL turned a setting down
 */
bool clock_init(void);

#endif
