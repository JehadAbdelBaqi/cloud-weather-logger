#ifndef POLLING_H
#define POLLING_H

#include <stdint.h>

// Asking the bridge whether it is there, on a timer, and noticing when it stops answering.
// Knows nothing about any line type: the line to ask with is handed in.

/**
 * @brief Sends a request to the bridge when one is due. Does not wait.
 *
 * Until the bridge answers, a request goes out every request interval, for as long as it takes.
 * Once the connection is made, nothing is sent while lines keep arriving from the bridge; after
 * the quiet limit with none, requests go out again at the same interval, and after the missed
 * limit in a row go unanswered the connection counts as lost.
 *
 * Call on every pass of the main loop.
 *
 * @param letter              The letter of the line to ask with, e.g. PROTOCOL_LETTER_HANDSHAKE
 * @param word                The word of the line to ask with, e.g. PROTOCOL_HANDSHAKE_REQUEST
 * @param quiet_limit_ms      How long nothing may be heard from the bridge before a request is due
 * @param request_interval_ms The time between one request and the next
 * @param missed_limit        How many requests in a row may go unanswered before the connection
 *                            is lost
 */
void polling_send_when_due(char letter, const char *word, uint32_t quiet_limit_ms, uint32_t request_interval_ms, int missed_limit);

/**
 * @brief Notes that a line has arrived from the bridge: it is still there, and nothing is missed.
 *
 * Any line counts, whatever its type. Does not make the connection.
 */
void polling_note_heard(void);

#endif
