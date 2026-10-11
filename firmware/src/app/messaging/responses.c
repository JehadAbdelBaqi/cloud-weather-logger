#include "responses.h"

#include <stdbool.h>
#include <stdint.h>

#include "debug.h"
#include "line.h"
#include "router.h"
#include "states.h"

// Room for the status of a response, with its ending '\0'
#define STATUS_SIZE 17

// Room for a sequence number as text: up to 10 digits, then the ending '\0'
#define SEQ_SIZE 11

/**
 * @brief Takes the status out of a response: L,<letter>,<ID>,<status>.
 *
 * A response with no status, or with a word that is not a status, is shown on the debug output.
 *
 * @param line   The response received
 * @param status Filled in with the status
 * @return true if the response had a status that is known
 */
static bool read_status(const char *line, status_t *status)
{
    char word[STATUS_SIZE];

    if (!line_copy_field(line, 2, word, sizeof(word))) {
        debug_print("response ignored: no status in it\r\n");
        return false;
    }
    if (!status_from_word(word, status)) {
        debug_print("response ignored: not a status that is known\r\n");
        return false;
    }
    return true;
}

/**
 * @brief Takes the sequence number out of the response to a reading: L,W,<seq>,<status>.
 *
 * @param line The response received
 * @param seq  Filled in with the sequence number
 * @return true if the response had a sequence number
 */
static bool read_seq(const char *line, uint32_t *seq)
{
    char text[SEQ_SIZE];

    if (!line_copy_field(line, 1, text, sizeof(text))) {
        debug_print("response ignored: no sequence number in it\r\n");
        return false;
    }
    if (!line_read_number(text, seq)) {
        debug_print("response ignored: its sequence number is not a number\r\n");
        return false;
    }
    return true;
}

void handle_handshake_response(const char *line)
{
    status_t status;

    if (!read_status(line, &status)) {
        return;
    }

    handshake_actions[status]();
}

void handle_reading_response(const char *line)
{
    uint32_t seq = 0;
    status_t status;

    if (!read_seq(line, &seq)) {
        return;
    }
    if (!read_status(line, &status)) {
        return;
    }

    reading_actions[status](seq);
}

void handle_reply_response(const char *line)
{
    (void)line;  // a placeholder: nothing is done with the response to a reply yet
}
