#ifndef BOARD_H
#define BOARD_H

#include "stm32f4xx_hal.h"

// Pin assignments on the Nucleo-F411RE.

// Green user LED (LD2) - PA5, Nucleo user manual UM1724 section 7.6
#define LED_PORT GPIOA
#define LED_PIN  GPIO_PIN_5

// Link to the Wi-Fi bridge - USART1, PA9 TX (Arduino D8) / PA10 RX (Arduino D2),
// alternate function 7 (STM32F411xE datasheet DS10314 Table 8, UM1724 Table 16)
#define LINK_UART    USART1
#define LINK_PORT    GPIOA
#define LINK_TX_PIN  GPIO_PIN_9
#define LINK_RX_PIN  GPIO_PIN_10
#define LINK_PIN_AF  GPIO_AF7_USART1
#define LINK_BAUD    115200

// Debug output - USART2 TX on PA2, which the ST-LINK passes to the PC as a USB serial port
// (UM1724 section 6.8); alternate function 7. Output only: no RX pin is set up.
#define DEBUG_UART    USART2
#define DEBUG_PORT    GPIOA
#define DEBUG_TX_PIN  GPIO_PIN_2
#define DEBUG_PIN_AF  GPIO_AF7_USART2
#define DEBUG_BAUD    115200

// BME280 sensor - I2C1, PB8 SCL (Arduino D15) / PB9 SDA (Arduino D14), alternate function 4
// (UM1724 Table 16, STM32F411xE datasheet DS10314 Table 9)
#define SENSOR_I2C           I2C1
#define SENSOR_PORT          GPIOB
#define SENSOR_SCL_PIN       GPIO_PIN_8
#define SENSOR_SDA_PIN       GPIO_PIN_9
#define SENSOR_PIN_AF        GPIO_AF4_I2C1
#define SENSOR_I2C_SPEED_HZ  100000

#endif
