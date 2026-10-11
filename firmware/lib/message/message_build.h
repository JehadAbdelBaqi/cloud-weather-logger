#ifndef MESSAGE_BUILD_H
#define MESSAGE_BUILD_H

#include <stddef.h>

// The most fields a message has after its letter
#define MESSAGE_MAX_FIELDS 5

// The fields of a message, in order, each one already text
typedef struct {
    const char *text[MESSAGE_MAX_FIELDS];
} fields_t;

// A message: its letter, how many fields it has after the letter, and the fields
typedef struct {
    char letter;
    size_t number_of_fields;
    fields_t fields;
} message_t;

// Described in message_build.c
void message_start(char *text, char letter);
void message_add_field(char *text, char separator, const char *field);
char *construct_message(char *text, char separator, message_t message);

#endif
