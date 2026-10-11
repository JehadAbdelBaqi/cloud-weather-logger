#ifndef ROUTING_H
#define ROUTING_H

#include <stdint.h>

#include "app_state.h"
#include "states.h"

// A function a route leads to: it is handed the program's running state
typedef void (*route_handler_t)(app_state_t *state);

// A function an incoming route leads to: it is handed the program's running state and the message
// that arrived
typedef void (*incoming_handler_t)(app_state_t *state, const char *text);

// How many positions the incoming table has: one for every character a letter can be
#define INCOMING_ROUTES_SIZE 128

// The table for messages coming in: the function for each letter sits at that letter's position
extern const incoming_handler_t incoming_routes[INCOMING_ROUTES_SIZE];

// The table for responses: the function for each second letter (the letter of the message the
// response answers) sits at that letter's position
extern const incoming_handler_t response_routes[INCOMING_ROUTES_SIZE];

// Described in routing.c
void call_route(const incoming_handler_t table[], char letter, app_state_t *state, const char *text);
void response_router(app_state_t *state, const char *text);

// The table of states: the function for each state sits at that state's position
extern const route_handler_t state_routing[STATE_COUNT];

#endif
