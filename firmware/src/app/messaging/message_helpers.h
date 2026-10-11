#ifndef MESSAGE_HELPERS_H
#define MESSAGE_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#include "format_number.h"
#include "message.h"

// Helpers for filling in a message before it is constructed, e.g. turning a number into text

// Described in message_helpers.c
void fill_message(message_t *message, const char *const values[]);
void numbers_to_text(const int32_t numbers[], size_t count, char texts[][FORMAT_SIGNED_SIZE]);
void fill_message_fields_from_numbers(message_t *message, const int32_t numbers[], char texts[][FORMAT_SIGNED_SIZE]);

#endif
