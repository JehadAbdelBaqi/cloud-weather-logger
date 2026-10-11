#include "state_machine.h"

#include "router.h"
#include "states.h"

// The state the Nucleo is in
static state_t state = STATE_NORMAL;

void state_machine_set_state(state_t new_state)
{
    state = new_state;
}

void state_machine_run(void)
{
    poll_bridge();  // every state has to hear what the bridge sends

    switch (state) {
    case STATE_NORMAL:
        state_normal();
        break;
    case STATE_BRIDGE_DOWN:
        state_bridge_down();
        break;
    }
}
