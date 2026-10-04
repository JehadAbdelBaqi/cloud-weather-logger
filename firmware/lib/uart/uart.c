#include "uart.h"

// Longest wait for one uart_write() to go out, in milliseconds
#define WRITE_TIMEOUT_MS 100

/**
 * @brief Turns on the clock of the GPIO port a UART's pins are on.
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

/**
 * @brief Turns on the clock of a UART.
 *
 * @param instance The UART
 * @return true if the clock was turned on, false if the UART is not one this driver knows
 */
static bool uart_clock_enable(const USART_TypeDef *instance)
{
    if (instance == USART1) {
        __HAL_RCC_USART1_CLK_ENABLE();
        return true;
    }
    if (instance == USART2) {
        __HAL_RCC_USART2_CLK_ENABLE();
        return true;
    }
    return false;
}

/**
 * @brief Hands the TX and RX pins to the UART.
 *
 * The pull-up holds RX at the idle level while nothing is connected to it.
 *
 * @param config Which port, pins and alternate function
 */
static void pins_init(const uart_config_t *config)
{
    GPIO_InitTypeDef pins = {0};
    pins.Pin = config->tx_pin | config->rx_pin;
    pins.Mode = GPIO_MODE_AF_PP;
    pins.Pull = GPIO_PULLUP;
    pins.Speed = GPIO_SPEED_FREQ_LOW;
    pins.Alternate = config->pin_af;
    HAL_GPIO_Init(config->port, &pins);
}

/**
 * @brief Sets the UART's speed and frame format (8N1) and switches it on.
 *
 * @param uart   Filled in by this call
 * @param config Which UART and its speed
 * @return true if the HAL accepted the settings
 */
static bool settings_init(uart_t *uart, const uart_config_t *config)
{
    uart->Instance = config->instance;
    uart->Init.BaudRate = config->baud;
    uart->Init.WordLength = UART_WORDLENGTH_8B;
    uart->Init.StopBits = UART_STOPBITS_1;
    uart->Init.Parity = UART_PARITY_NONE;
    uart->Init.Mode = UART_MODE_TX_RX;
    uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart->Init.OverSampling = UART_OVERSAMPLING_16;
    return HAL_UART_Init(uart) == HAL_OK;
}

bool uart_init(uart_t *uart, const uart_config_t *config)
{
    if (!port_clock_enable(config->port)) {
        return false;
    }
    if (!uart_clock_enable(config->instance)) {
        return false;
    }
    pins_init(config);
    return settings_init(uart, config);
}

bool uart_write(uart_t *uart, const uint8_t *data, size_t length)
{
    return HAL_UART_Transmit(uart, data, length, WRITE_TIMEOUT_MS) == HAL_OK;
}

bool uart_read_byte(uart_t *uart, uint8_t *byte)
{
    return HAL_UART_Receive(uart, byte, 1, 0) == HAL_OK;
}
