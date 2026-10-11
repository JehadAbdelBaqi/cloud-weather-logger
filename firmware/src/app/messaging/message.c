#include "message.h"

#include <string.h>

#include "protocol.h"

/**
 * @brief Starts a message's text: its type's letter and nothing after it.
 *
 * @param text   Filled in with the text so far
 * @param letter The letter of the message's type, e.g. PROTOCOL_LETTER_READING
 */
void message_start(char *text, char letter)
{
    text[0] = letter;
    text[1] = '\0';
}

/**
 * @brief Adds a field to the end of a message's text: a separator, then the field.
 *
 * Does not check that the field fits: the caller makes sure the buffer has room for it.
 *
 * @param text  The text so far
 * @param field The field's text
 */
void message_add_field(char *text, const char *field)
{
    size_t used = strlen(text);
    size_t length = strlen(field);

    text[used] = PROTOCOL_FIELD_SEPARATOR;
    memcpy(&text[used + 1], field, length + 1);
}

/**
 * @brief Constructs a message as text: its letter, then each of its fields, in order.
 *
 * Every field is text: a number is turned into text before it goes into the message.
 *
 * @param text    The buffer the text is built in
 * @param message The message: its letter, how many fields it has and the fields
 * @return The text: the same buffer that was handed in
 */
char *construct_message(char *text, message_t message)
{
    message_start(text, message.letter);
    for (size_t i = 0; i < message.number_of_fields; i++) {
        message_add_field(text, message.fields.text[i]);
    }
    return text;
}
