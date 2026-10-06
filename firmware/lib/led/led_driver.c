#include "led_driver.h"

#include "gpio_driver.h"

static GPIO_TypeDef *led_port;
static uint16_t led_pin;

bool led_init(GPIO_TypeDef *port, uint16_t pin)
{
    led_port = port;
    led_pin = pin;
    return gpio_init_output_pin(led_port, led_pin);
}

void led_toggle(void)
{
    HAL_GPIO_TogglePin(led_port, led_pin);
}
