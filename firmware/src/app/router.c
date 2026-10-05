#include "router.h"

#include <stddef.h>

#include "debug.h"
#include "handshake.h"
#include "link.h"

void router_poll(void)
{
    const char *line = link_read_line();
    if (line == NULL) {
        return;
    }

    if (handshake_handle_line(line)) {
        return;
    }

    debug_print("down: ");
    debug_print(line);
    debug_print("\r\n");
}
