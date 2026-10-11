#ifndef STATES_H
#define STATES_H

// The states. Described in states.c

// The keys for the states: names for the positions in the state table
typedef enum {
    STATE_OK,           // everything is answering
    STATE_NO_RESPONSE,  // a message went out and no response came back in time
    STATE_COUNT,        // how many states there are
} state_key_t;

// The program's running state: defined in app_state.h, which includes this file for the keys
struct app_state;

void state_ok(struct app_state *state);
void state_no_response(struct app_state *state);
void state_setter(struct app_state *state);

#endif
