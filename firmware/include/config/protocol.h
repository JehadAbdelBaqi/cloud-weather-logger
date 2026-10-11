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
#define PROTOCOL_LETTER_RESPONSE  'L'  // down: a response to a line sent up, from the bridge or the server

// How many fields each line type sent up has, not counting its letter
#define PROTOCOL_FIELDS_READING   5  // seq, time, temperature, pressure, humidity
#define PROTOCOL_FIELDS_REPLY     2  // id, result
#define PROTOCOL_FIELDS_HANDSHAKE 1  // the word

// Handshake: H,<word>
#define PROTOCOL_HANDSHAKE_REQUEST "request"  // "are you there?"

// Command: C,<id>,<command>, where the command is one of these words, with a value after "rate"
#define PROTOCOL_COMMAND_HELP "help"  // lists the commands
#define PROTOCOL_COMMAND_READ "read"  // takes a reading now
#define PROTOCOL_COMMAND_RATE "rate"  // sets how often readings are taken: rate <seconds>

// Response: L,<letter>,<ID>,<status>, with the letter and first field of the line it is a
// response to. The status is PROTOCOL_REPLY_OK (the broker has the line) or one of these. For the
// response to a handshake request it says how far the bridge can reach:
// PROTOCOL_REPLY_OK, or one of these
#define PROTOCOL_RESPONSE_NO_WIFI   "con_err_w"   // the bridge has no Wi-Fi
#define PROTOCOL_RESPONSE_NO_BROKER "con_err_br"  // the bridge has Wi-Fi, but no connection to the broker
#define PROTOCOL_RESPONSE_BROKER_ERR "err_br"      // the broker did not confirm the line in time
#define PROTOCOL_RESPONSE_TOO_LONG  "ans_err_long"  // the bridge's line was too long to send: the first field is left empty

// Reply: R,<id>,<result>, where the result is one of these words or free text
#define PROTOCOL_REPLY_OK  "ok"
#define PROTOCOL_REPLY_ERR "err"  // followed by a space and the reason

#endif
