#include "router.h"

#include <stddef.h>

#include "debug.h"
#include "link.h"
#include "message_handlers.h"
#include "polling.h"
#include "protocol.h"

void router_poll(void)
{
    const char *line = link_read_line();
    if (line == NULL) {
        return;
    }

    if (line[0] == PROTOCOL_LETTER_HANDSHAKE) {
        handle_handshake(line);
        return;
    }

    polling_note_heard();
    debug_print("down: ");
    debug_print(line);
    debug_print("\r\n");
}
