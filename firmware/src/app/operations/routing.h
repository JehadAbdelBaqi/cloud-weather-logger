#ifndef ROUTING_H
#define ROUTING_H

#include <stdint.h>

// A function a route leads to
typedef void (*route_handler_t)(void);

// A function an incoming route leads to: it is handed the message that arrived
typedef void (*incoming_handler_t)(const char *text);

// How many positions the incoming table has: one for every character a letter can be
#define INCOMING_ROUTES_SIZE 128

// The table for messages coming in: the function for each letter sits at that letter's position
extern const incoming_handler_t incoming_routes[INCOMING_ROUTES_SIZE];

// The table for responses: the function for each second letter (the letter of the message the
// response answers) sits at that letter's position
extern const incoming_handler_t response_routes[INCOMING_ROUTES_SIZE];

// The keys for the states: names for the positions in the state table
typedef enum {
    STATE_OK,           // everything is answering
    STATE_NO_RESPONSE,  // a message went out and no response came back in time
    STATE_COUNT,        // how many states there are
} state_key_t;

// The table of states: the function for each state sits at that state's position
extern const route_handler_t state_routing[STATE_COUNT];

#endif
