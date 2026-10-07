#ifndef POLLING_H
#define POLLING_H

// Asking the bridge whether it is there, on a timer, and noticing when it stops answering.
// Knows nothing about any line type: the line to ask with is handed in.

/**
 * @brief Sends a request to the bridge when one is due. Does not wait.
 *
 * Until the bridge answers, a request goes out every HANDSHAKE_REQUEST_INTERVAL_MS, for as long as
 * it takes. Once the connection is made, nothing is sent while lines keep arriving from the bridge;
 * after HANDSHAKE_QUIET_LIMIT_MS with none, requests go out again at the same interval, and after
 * HANDSHAKE_MISSED_LIMIT in a row go unanswered the connection counts as lost.
 *
 * Call on every pass of the main loop.
 *
 * @param letter The letter of the line to ask with, e.g. PROTOCOL_LETTER_HANDSHAKE
 * @param word   The word of the line to ask with, e.g. PROTOCOL_HANDSHAKE_REQUEST
 */
void polling_send_when_due(char letter, const char *word);

/**
 * @brief Notes that a line has arrived from the bridge: it is still there, and nothing is missed.
 *
 * Any line counts, whatever its type. Does not make the connection.
 */
void polling_note_heard(void);

#endif
