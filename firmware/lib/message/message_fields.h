#ifndef MESSAGE_FIELDS_H
#define MESSAGE_FIELDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "format_number.h"
#include "message_build.h"

// The fields of a message: filling them in before the message is constructed, e.g. turning a
// number into text, and getting one out of a message that has arrived

// Described in message_fields.c
void fill_message(message_t *message, const char *const values[]);
void numbers_to_text(const int32_t numbers[], size_t count, char texts[][FORMAT_SIGNED_SIZE]);
void fill_message_fields_from_numbers(message_t *message, const int32_t numbers[], char texts[][FORMAT_SIGNED_SIZE]);
bool get_field(const char *text, char separator, size_t index, char *field, size_t size);

#endif
