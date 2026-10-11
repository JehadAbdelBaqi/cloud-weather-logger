#include "messages.h"

#include "protocol.h"

// The handshake message: H,request
const message_t handshake_message = {
    .letter = PROTOCOL_LETTER_HANDSHAKE,
    .number_of_fields = PROTOCOL_FIELDS_HANDSHAKE,
    .fields = { .text = { PROTOCOL_HANDSHAKE_REQUEST } },
};

// The reading message: W,<seq>,<time>,<temperature>,<pressure>,<humidity>.
// The fields are empty here: they are filled in, as text, for each reading that is sent
message_t reading_message = {
    .letter = PROTOCOL_LETTER_READING,
    .number_of_fields = PROTOCOL_FIELDS_READING,
};
