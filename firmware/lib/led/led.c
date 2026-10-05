#include "led.h"

static GPIO_TypeDef *led_port;
static uint16_t led_pin;

/**
 * @brief Turns on the clock of the GPIO port the LED is on.
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
    return false;
}

bool led_init(GPIO_TypeDef *port, uint16_t pin)
{
    if (!port_clock_enable(port)) {
        return false;
    }
    led_port = port;
    led_pin = pin;

    GPIO_InitTypeDef settings = {0};
    settings.Pin = led_pin;
    settings.Mode = GPIO_MODE_OUTPUT_PP;
    settings.Pull = GPIO_NOPULL;
    settings.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(led_port, &settings);

    HAL_GPIO_WritePin(led_port, led_pin, GPIO_PIN_RESET);
    return true;
}

void led_toggle(void)
{
    HAL_GPIO_TogglePin(led_port, led_pin);
}
