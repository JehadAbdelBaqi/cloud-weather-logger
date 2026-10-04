#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Sets up the debug output: text sent to the PC's serial monitor through the ST-LINK.
 *
 * Call once before debug_print().
 *
 * @return true if the debug UART was set up
 */
bool debug_init(void);

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

#endif
