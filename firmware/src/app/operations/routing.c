#include "routing.h"

#include <stddef.h>

#include "config.h"
#include "protocol.h"
#include "handlers.h"
#include "incoming.h"
#include "outgoing.h"
#include "states.h"

// The table for messages coming in: the function for each letter sits at that letter's position.
const incoming_handler_t incoming_routes[INCOMING_ROUTES_SIZE] = {
    [PROTOCOL_LETTER_RESPONSE] = response_router,
    [PROTOCOL_LETTER_COMMAND]  = handle_incoming_command,
};

// The table for responses: the function for each second letter sits at that letter's position.
const incoming_handler_t response_routes[INCOMING_ROUTES_SIZE] = {
    [PROTOCOL_LETTER_HANDSHAKE] = handle_handshake_ack,
    [PROTOCOL_LETTER_READING]   = handle_weather_reading_ack,
    [PROTOCOL_LETTER_REPLY]     = handle_reply_ack,
};

// The table of states: the function for each state sits at that state's position.
const route_handler_t state_routing[STATE_COUNT] = {
    [STATE_OK]          = state_ok,
    [STATE_NO_RESPONSE] = state_no_response,
};
