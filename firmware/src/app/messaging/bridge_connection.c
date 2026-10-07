#include "bridge_connection.h"

#include "debug.h"

static bool is_connected = false;

void bridge_connection_set_connected(bool now_connected)
{
    bool changed = (now_connected != is_connected);

    is_connected = now_connected;

    if (!changed) {
        return;
    }
    if (now_connected) {
        debug_print("bridge connected\r\n");
        return;
    }
    debug_print("bridge lost: waiting for the bridge...\r\n");
}

bool bridge_connection_is_connected(void)
{
    return is_connected;
}
