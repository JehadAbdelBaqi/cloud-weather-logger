#ifndef LINE_H
#define LINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "link_framing.h"

// Building and reading protocol lines as text: a letter, then fields, each with a separator
// before it (protocol.h). Nothing here sends or receives anything.

// Room for any line: the longest one, then its ending '\0'
#define LINE_BUFFER_SIZE (LINK_LINE_MAX_LEN + 1)

/**
 * @brief Starts a line: its type's letter and nothing after it.
 *
 * @param line   Filled in with the line so far; needs LINE_BUFFER_SIZE characters
 * @param letter The letter of the line's type, e.g. PROTOCOL_LETTER_READING
 */
void line_start(char *line, char letter);

/**
 * @brief Adds a field to the end of a line: a separator, then the text.
 *
 * @param line The line so far, in a buffer of LINE_BUFFER_SIZE characters
 * @param text The field's text
 * @return true if it was added; false if the line would be too long, and then nothing is added
 */
bool line_add_field(char *line, const char *text);

/**
 * @brief Adds a field to the end of a line: a separator, then a whole number in decimal.
 *
 * @param line  The line so far, in a buffer of LINE_BUFFER_SIZE characters
 * @param value The number
 * @return true if it was added; false if the line would be too long, and then nothing is added
 */
bool line_add_number(char *line, uint32_t value);

/**
 * @brief Adds a field to the end of a line: a separator, then a whole number that may be negative.
 *
 * @param line  The line so far, in a buffer of LINE_BUFFER_SIZE characters
 * @param value The number
 * @return true if it was added; false if the line would be too long, and then nothing is added
 */
bool line_add_signed(char *line, int32_t value);

/**
 * @brief Finds a field in a line.
 *
 * The field is not copied and has no '\0' of its own: the pointer returned runs on to the end
 * of the line, and length says where this field stops.
 *
 * @param line   The line, ending in '\0'
 * @param index  Which field: 0 is the first one after the letter
 * @param length Filled in with the field's length in characters, which may be 0
 * @return Where the field starts, or NULL if the line has no such field
 */
const char *line_field(const char *line, size_t index, size_t *length);

/**
 * @brief Says whether a line carries a given word, and nothing else, after its letter.
 *
 * Works for a line of any type: "H,ack" carries "ack", and so would "X,ack".
 *
 * @param line The line, ending in '\0'
 * @param word The word to look for
 * @return true if everything after the line's first separator is exactly that word
 */
bool message_word_is(const char *line, const char *word);

#endif
