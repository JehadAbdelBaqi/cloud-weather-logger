#ifndef MESSAGE_HELPERS_H
#define MESSAGE_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

#include "reading_store.h"

// Sending: one function for each kind of line the Nucleo sends, and the time to put on a reading.

/**
 * @brief Sends a line of one word to the bridge: the letter given, then the word.
 *
 * @param letter The letter of the line's type, e.g. PROTOCOL_LETTER_HANDSHAKE
 * @param word   The word, e.g. PROTOCOL_HANDSHAKE_REQUEST
 */
void send_word(char letter, const char *word);

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
 * @brief Sends a stored reading to the bridge as a reading line: W,<seq>,<epoch>,<t>,<p>,<rh>.
 *
 * The sequence number and the time are the ones the reading was stored with. It does not check
 * the connection to the bridge: it is for a caller that only runs while the bridge is connected.
 *
 * @param stored The reading, as taken from the store
 * @return true if the line was sent
 */
bool send_reading(const stored_reading_t *stored);

/**
 * @brief Gives the time to put on a reading taken now.
 *
 * A stand-in until the RTC is set up: a fixed starting time plus the seconds since power-on.
 *
 * @return Unix time in seconds
 */
uint32_t reading_time(void);

#endif
