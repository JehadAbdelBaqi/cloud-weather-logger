#ifndef FORMAT_NUMBER_H
#define FORMAT_NUMBER_H

#include <stdbool.h>
#include <stdint.h>

// Room each function needs for its text, counting the ending '\0'
#define FORMAT_NUMBER_SIZE   11  // 10 digits at most
#define FORMAT_SIGNED_SIZE   12  // a '-' and 10 digits at most
#define FORMAT_HEX_BYTE_SIZE 5   // "0x" and 2 digits

/**
 * @brief Writes a whole number as decimal text, e.g. "102137".
 *
 * @param value The number
 * @param text  Filled in with the text, ending in '\0'; needs FORMAT_NUMBER_SIZE characters
 */
void format_number(uint32_t value, char *text);

/**
 * @brief Writes a whole number that may be negative as decimal text, e.g. "-512".
 *
 * @param value The number
 * @param text  Filled in with the text, ending in '\0'; needs FORMAT_SIGNED_SIZE characters
 */
void format_signed(int32_t value, char *text);

/**
 * @brief Writes one byte as hexadecimal text, e.g. "0x60".
 *
 * @param value The byte
 * @param text  Filled in with the text, ending in '\0'; needs FORMAT_HEX_BYTE_SIZE characters
 */
void format_hex_byte(uint8_t value, char *text);

/**
 * @brief Reads a whole number written as decimal text, e.g. "102137": the reverse of
 * format_number().
 *
 * The text has to be digits and nothing else, and the number has to fit in 32 bits.
 *
 * @param text  The text, ending in '\0'
 * @param value Filled in with the number; left as it was if the text is not a number
 * @return true if the text was a number
 */
bool format_read_number(const char *text, uint32_t *value);

#endif
