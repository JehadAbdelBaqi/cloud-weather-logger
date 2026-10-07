#ifndef MESSAGE_HELPERS_H
#define MESSAGE_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

// What the message handlers share that touches the hardware: the link and the clock.

/**
 * @brief Sends a line of one word to the bridge: the letter given, then the word.
 *
 * @param letter The letter of the line's type, e.g. PROTOCOL_LETTER_HANDSHAKE
 * @param word   The word, e.g. PROTOCOL_HANDSHAKE_ACK
 */
void send_word(char letter, const char *word);

/**
 * @brief Sends a handshake line to the bridge: H, then the word.
 *
 * It is sent whether or not the connection is made, because the handshake is what makes it.
 *
 * @param word The word: PROTOCOL_HANDSHAKE_REQUEST or PROTOCOL_HANDSHAKE_ACK
 */
void send_handshake(const char *word);

/**
 * @brief Sends the reply to a command to the bridge: R,<id>,<result>.
 *
 * Nothing is sent while the connection to the bridge is not made.
 *
 * @param id     The ID of the command being answered
 * @param result The result: PROTOCOL_REPLY_OK, PROTOCOL_REPLY_ERR and a reason, or free text
 * @return true if the line was sent
 */
bool send_reply(const char *id, const char *result);

/**
 * @brief Gives the time to put on a reading taken now.
 *
 * A stand-in until the RTC is set up: a fixed starting time plus the seconds since power-on.
 *
 * @return Unix time in seconds
 */
uint32_t reading_time(void);

#endif
