#ifndef READING_TO_MESSAGE_H
#define READING_TO_MESSAGE_H

#include <stdint.h>

#include "format_number.h"
#include "message_build.h"
#include "protocol.h"
#include "reading.h"
#include "reading_store.h"

// A stored reading as a message: the one place that knows both what a reading is and what a
// message is

// How many values every reading sends after its first field: time, temperature, pressure and
// humidity. A reading message's fields without its sequence number
#define READING_VALUE_COUNT (PROTOCOL_FIELDS_READING - 1)

// Described in reading_to_message.c
void reading_values_to_text(uint32_t time, const reading_t *reading, char texts[][FORMAT_SIGNED_SIZE]);
void reading_to_message(const stored_reading_t *stored, message_t *message, char texts[][FORMAT_SIGNED_SIZE]);

#endif
