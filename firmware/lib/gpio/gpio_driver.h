#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

/**
 * @brief Hands pins to a peripheral, such as a UART or an I2C bus.
 *
 * Turns on the port's clock first. The internal pull-ups are switched on.
 *
 * @param port   GPIO port the pins are on, e.g. GPIOA
 * @param pins   The pins, e.g. GPIO_PIN_9 | GPIO_PIN_10
 * @param mode   How the pins are driven: GPIO_MODE_AF_PP (push-pull) or GPIO_MODE_AF_OD (open drain)
 * @param pin_af Alternate function that connects the pins to the peripheral
 * @return true if the pins were set up, false if the port is not one this driver knows
 */
bool gpio_init_peripheral_pins(GPIO_TypeDef *port, uint32_t pins, uint32_t mode, uint32_t pin_af);

/**
 * @brief Sets up a pin as an output, driven low.
 *
 * Turns on the port's clock first.
 *
 * @param port GPIO port the pin is on, e.g. GPIOA
 * @param pin  The pin, e.g. GPIO_PIN_5
 * @return true if the pin was set up, false if the port is not one this driver knows
 */
bool gpio_init_output_pin(GPIO_TypeDef *port, uint16_t pin);

#endif
