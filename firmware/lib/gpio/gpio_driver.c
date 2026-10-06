#include "gpio_driver.h"

/**
 * @brief Turns on the clock of a GPIO port.
 *
 * @param port The GPIO port
 * @return true if the clock was turned on, false if the port is not one this driver knows
 */
static bool port_clock_enable(const GPIO_TypeDef *port)
{
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        return true;
    }
    if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        return true;
    }
    if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        return true;
    }
    return false;
}

bool gpio_init_peripheral_pins(GPIO_TypeDef *port, uint32_t pins, uint32_t mode, uint32_t pin_af)
{
    if (!port_clock_enable(port)) {
        return false;
    }

    GPIO_InitTypeDef settings = {0};
    settings.Pin = pins;
    settings.Mode = mode;
    settings.Pull = GPIO_PULLUP;
    settings.Speed = GPIO_SPEED_FREQ_LOW;
    settings.Alternate = pin_af;
    HAL_GPIO_Init(port, &settings);
    return true;
}

bool gpio_init_output_pin(GPIO_TypeDef *port, uint16_t pin)
{
    if (!port_clock_enable(port)) {
        return false;
    }

    GPIO_InitTypeDef settings = {0};
    settings.Pin = pin;
    settings.Mode = GPIO_MODE_OUTPUT_PP;
    settings.Pull = GPIO_NOPULL;
    settings.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &settings);

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    return true;
}
