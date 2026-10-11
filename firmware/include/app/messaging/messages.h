#ifndef MESSAGES_H
#define MESSAGES_H

#include "message_build.h"

// The message types: one message_t for each kind of line the Nucleo sends

// The handshake message: H,request
extern const message_t handshake_message;

// The reading message: W,<seq>,<time>,<temperature>,<pressure>,<humidity>.
// Its fields are empty: they are filled in, as text, for each reading that is sent
extern message_t reading_message;

#endif
