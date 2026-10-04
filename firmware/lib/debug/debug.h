#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>

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

#endif
