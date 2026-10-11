#include "states.h"

#include "state_functions.h"

// The state each status leads to: the status is the place in the list
static const state_t status_states[] = {
    [STATUS_OK] = STATE_NORMAL,
};

state_t state_for_status(status_t status)
{
    return status_states[status];
}

void state_normal(void)
{
    send_stored_readings();
    read_and_store();
    send_oldest_reading();
}

void state_bridge_down(void)
{
    read_and_store();
    request_handshake();
}
