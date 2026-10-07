#include "message_handlers.h"

#include <stddef.h>
#include <stdint.h>

#include "bridge_connection.h"
#include "commands.h"
#include "debug.h"
#include "line.h"
#include "link.h"
#include "message_helpers.h"
#include "polling.h"
#include "protocol.h"

// Room for the ID of a command: up to 16 characters, then the ending '\0'
#define COMMAND_ID_SIZE 17

void handle_handshake(const char *line)
{
    polling_note_heard();

    if (message_word_is(line, PROTOCOL_HANDSHAKE_REQUEST)) {
        send_handshake(PROTOCOL_HANDSHAKE_ACK);
        send_handshake(PROTOCOL_HANDSHAKE_REQUEST);
        return;
    }
    if (message_word_is(line, PROTOCOL_HANDSHAKE_ACK)) {
        bridge_connection_set_connected(true);
        return;
    }

    debug_print("line ignored: not a handshake line\r\n");
}

void handle_command(const char *line)
{
    char id[COMMAND_ID_SIZE];
    size_t length = 0;
    const char *command;

    polling_note_heard();

    if (!line_copy_field(line, 0, id, sizeof(id))) {
        debug_print("command ignored: no ID, or one that is too long\r\n");
        return;
    }
    if (id[0] == '\0') {
        debug_print("command ignored: no ID, or one that is too long\r\n");
        return;
    }

    command = line_field(line, 1, &length);
    if (command == NULL) {
        send_reply(id, PROTOCOL_REPLY_ERR " no command");
        return;
    }

    commands_run(id, command);
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
