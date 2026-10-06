#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

/**
 * @brief Sets up the pin of an LED as an output, LED off.
 *
 * Call once before led_toggle().
 *
 * @param port GPIO port the LED is on, e.g. GPIOA
 * @param pin  Its pin, e.g. GPIO_PIN_5
 * @return true if the pin was set up, false if the port is not one this driver knows
 */
bool led_init(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Switches the LED on if it is off, and off if it is on.
 */
void led_toggle(void);

#endif
