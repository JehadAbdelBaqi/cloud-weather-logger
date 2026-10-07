#ifndef MESSAGE_HELPERS_H
#define MESSAGE_HELPERS_H

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
 * @brief Gives the time to put on a reading taken now.
 *
 * A stand-in until the RTC is set up: a fixed starting time plus the seconds since power-on.
 *
 * @return Unix time in seconds
 */
uint32_t reading_time(void);

#endif
