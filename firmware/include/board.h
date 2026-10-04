#ifndef BOARD_H
#define BOARD_H

#include "stm32f4xx_hal.h"

// Pin assignments on the Nucleo-F411RE.

// Green user LED (LD2) - PA5, Nucleo user manual UM1724 section 7.6
#define LED_PORT GPIOA
#define LED_PIN  GPIO_PIN_5

#endif
