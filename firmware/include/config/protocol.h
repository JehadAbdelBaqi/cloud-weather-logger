#ifndef PROTOCOL_H
#define PROTOCOL_H

// The lines exchanged with the bridge: the letter of each line type, how a line is laid out and
// the fixed words used in it. The protocol is described in docs/system/protocol.md.

// A line is its type's letter, then its fields, each with a separator before it: R,7,ok
#define PROTOCOL_FIELD_SEPARATOR ','

// Line types: the first letter of a line
#define PROTOCOL_LETTER_READING   'W'  // up: a reading
#define PROTOCOL_LETTER_REPLY     'R'  // up: the reply to a command
#define PROTOCOL_LETTER_COMMAND   'C'  // down: a command
#define PROTOCOL_LETTER_HANDSHAKE 'H'  // both ways, between the Nucleo and the bridge only

// Handshake: H,<word>
#define PROTOCOL_HANDSHAKE_REQUEST "request"  // "are you there?"
#define PROTOCOL_HANDSHAKE_ACK     "ack"      // "I am here"

// Command: C,<id>,<command>, where the command is one of these words, with a value after "rate"
#define PROTOCOL_COMMAND_HELP "help"  // lists the commands
#define PROTOCOL_COMMAND_READ "read"  // takes a reading now
#define PROTOCOL_COMMAND_RATE "rate"  // sets how often readings are taken: rate <seconds>

// Reply: R,<id>,<result>, where the result is one of these words or free text
#define PROTOCOL_REPLY_OK  "ok"
#define PROTOCOL_REPLY_ERR "err"  // followed by a space and the reason

#endif
