#ifndef READING_TO_MESSAGE_H
#define READING_TO_MESSAGE_H

#include "format_number.h"
#include "message.h"
#include "reading_store.h"

// A stored reading as a message: the one place that knows both what a reading is and what a
// message is

// Described in reading_to_message.c
void reading_to_message(const stored_reading_t *stored, message_t *message, char texts[][FORMAT_SIGNED_SIZE]);

#endif
