#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "states.h"

// The state machine: holds the state the Nucleo is in, and runs that state's function.

/**
 * @brief Puts the Nucleo in a state. The next run of the machine runs that state's function.
 *
 * The only way the state is changed.
 *
 * @param new_state The state to be in
 */
void state_machine_set_state(state_t new_state);

/**
 * @brief Takes in a line from the bridge if one has arrived, then runs the state the Nucleo is
 *        in, once.
 *
 * Call on every pass. Does not wait.
 */
void state_machine_run(void);

#endif
