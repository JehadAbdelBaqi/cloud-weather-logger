#include "message_helpers.h"

/**
 * @brief Fills in a message's fields with the values given, in order.
 *
 * The message says how many fields it has, and the list has to hold that many values.
 *
 * @param message The message to fill in
 * @param values  The values, as text, in the order of the message's fields
 */
void fill_message(message_t *message, const char *const values[])
{
    for (size_t i = 0; i < message->number_of_fields; i++) {
        message->fields.text[i] = values[i];
    }
}

/**
 * @brief Turns a list of numbers into text, one text for each number, in the same order.
 *
 * @param numbers The numbers, which may be negative
 * @param count   How many numbers are in the list
 * @param texts   Filled in with the texts: one buffer of FORMAT_SIGNED_SIZE characters for
 *                each number
 */
void numbers_to_text(const int32_t numbers[], size_t count, char texts[][FORMAT_SIGNED_SIZE])
{
    for (size_t i = 0; i < count; i++) {
        format_signed(numbers[i], texts[i]);
    }
}

/**
 * @brief Fills in a message whose fields are all numbers: turns each number into text, then
 *        puts the texts into the message's fields.
 *
 * The message says how many fields it has, and the list has to hold that many numbers.
 *
 * @param message The message to fill in: the caller's own, e.g. its copy of reading_message
 * @param numbers The numbers, in the order of the message's fields
 * @param texts   Buffers for the texts, one of FORMAT_SIGNED_SIZE characters for each number.
 *                The message's fields point into them, so they have to last as long as the
 *                message is used
 */
void fill_message_fields_from_numbers(message_t *message, const int32_t numbers[], char texts[][FORMAT_SIGNED_SIZE])
{
    const char *values[MESSAGE_MAX_FIELDS];

    numbers_to_text(numbers, message->number_of_fields, texts);
    for (size_t i = 0; i < message->number_of_fields; i++) {
        values[i] = texts[i];
    }
    fill_message(message, values);
}
