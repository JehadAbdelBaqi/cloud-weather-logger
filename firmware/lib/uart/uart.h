#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

/** One UART, once uart_init() has set it up. */
typedef UART_HandleTypeDef uart_t;

/** Which UART to set up, the pins it uses and its speed. Always 8 data bits, no parity, 1 stop bit. */
typedef struct {
    USART_TypeDef *instance;  // the UART itself, e.g. USART1
    GPIO_TypeDef *port;       // GPIO port both pins are on
    uint32_t tx_pin;          // e.g. GPIO_PIN_9
    uint32_t rx_pin;          // e.g. GPIO_PIN_10
    uint32_t pin_af;          // alternate function that connects the pins to the UART
    uint32_t baud;            // bits per second
} uart_config_t;

/**
 * @brief Sets up a UART for sending and receiving bytes.
 *
 * @param uart   Filled in by this call; pass it to the other uart_ functions
 * @param config Which UART, its pins and its speed
 * @return true if the UART was set up, false if the UART or port is not one this driver knows,
 *         or the HAL turned a setting down
 */
bool uart_init(uart_t *uart, const uart_config_t *config);

/**
 * @brief Sends bytes and waits until they have gone out.
 *
 * @param uart   A UART set up with uart_init()
 * @param data   The bytes to send
 * @param length How many bytes
 * @return true if every byte was sent, false if it timed out
 */
bool uart_write(uart_t *uart, const uint8_t *data, size_t length);

#endif
