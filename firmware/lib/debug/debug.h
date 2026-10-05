#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdint.h>

#include "uart.h"

/**
 * @brief Sets up the debug output: text sent to the PC's serial monitor through the ST-LINK.
 *
 * Call once before debug_print().
 *
 * @param config Which UART the debug output is on, its pins and its speed
 * @return true if the debug UART was set up
 */
bool debug_init(const uart_config_t *config);

/**
 * @brief Writes text to the debug output, unchanged.
 *
 * Nothing is added: to end a line, end the text with "\r\n".
 *
 * @param text Text to write, '\0'-terminated
 */
void debug_print(const char *text);

/**
 * @brief Writes one byte to the debug output as hexadecimal, e.g. "0x60".
 *
 * @param value The byte
 */
void debug_print_hex(uint8_t value);

/**
 * @brief Writes a whole number to the debug output in decimal.
 *
 * @param value The number
 */
void debug_print_number(uint32_t value);

/**
 * @brief Writes a whole number that may be negative to the debug output in decimal.
 *
 * @param value The number
 */
void debug_print_signed(int32_t value);

#endif
