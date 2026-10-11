#include "state_functions.h"

#include <stdbool.h>

#include "bridge_connection.h"
#include "message_helpers.h"
#include "polling.h"
#include "protocol.h"
#include "reading.h"
#include "reading_store.h"
#include "sensor_timer.h"
#include "state_machine.h"
#include "states.h"

void read_and_store(void)
{
    reading_t reading;

    if (!sensor_poll(&reading)) {
        return;  // not time for a reading yet
    }
    reading_store_put(&reading, reading_time());
}

void send_oldest_reading(void)
{
    stored_reading_t stored;

    if (!reading_store_oldest(&stored)) {
        return;  // nothing waiting to be sent
    }
    send_reading(&stored);
}

void send_stored_readings(void)
{
    while (!reading_store_is_empty()) {
        send_oldest_reading();

        // TODO: HANDLE responses. A good one removes the reading from the store and the loop goes
        // round again; a bad one breaks out of the loop.
        break;
    }
}

void request_handshake(void)
{
    polling_send_when_due(PROTOCOL_LETTER_HANDSHAKE, PROTOCOL_HANDSHAKE_REQUEST);
}

void connect_bridge(void)
{
    bridge_connection_set_connected(true);
    state_machine_set_state(state_for_status(STATUS_OK));
}

void confirm_reading(uint32_t seq)
{
    reading_store_remove(seq);
    state_machine_set_state(state_for_status(STATUS_OK));
}
