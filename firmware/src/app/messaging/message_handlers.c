#include "message_handlers.h"

#include <stdint.h>

#include "bridge_connection.h"
#include "debug.h"
#include "line.h"
#include "link.h"
#include "message_helpers.h"
#include "polling.h"
#include "protocol.h"

void handle_handshake(const char *line)
{
    polling_note_heard();

    if (message_word_is(line, PROTOCOL_HANDSHAKE_REQUEST)) {
        send_word(PROTOCOL_LETTER_HANDSHAKE, PROTOCOL_HANDSHAKE_ACK);
        send_word(PROTOCOL_LETTER_HANDSHAKE, PROTOCOL_HANDSHAKE_REQUEST);
        return;
    }
    if (message_word_is(line, PROTOCOL_HANDSHAKE_ACK)) {
        bridge_connection_set_connected(true);
        return;
    }

    debug_print("line ignored: not a handshake line\r\n");
}

bool handle_reading(const reading_t *reading)
{
    static uint32_t seq = 0;
    char line[LINE_BUFFER_SIZE];

    if (!bridge_connection_is_connected()) {
        return false;
    }
    seq++;

    line_start(line, PROTOCOL_LETTER_READING);
    line_add_number(line, seq);
    line_add_number(line, reading_time());
    line_add_signed(line, reading->temperature);
    line_add_number(line, reading->pressure);
    line_add_number(line, reading->humidity);

    return link_send_line(line);
}
