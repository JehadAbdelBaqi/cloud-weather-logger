#include "polling.h"

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

#include "bridge_connection.h"
#include "config.h"
#include "message_helpers.h"
#include "time_interval.h"

static bool awaiting_ack = false;    // a request has been sent and not answered yet
static int missed = 0;               // requests in a row that went unanswered
static bool request_sent = false;    // false until the first request has gone out
static uint32_t last_request_ms = 0;
static uint32_t last_heard_ms = 0;   // when the last line arrived from the bridge

void polling_note_heard(void)
{
    last_heard_ms = HAL_GetTick();
    missed = 0;
    awaiting_ack = false;
}

/**
 * @brief Notes that the last request went unanswered; after the limit, the connection is lost.
 */
static void note_missed(void)
{
    if (!bridge_connection_is_connected()) {
        return;  // still trying to make the connection: nothing to lose
    }

    missed++;
    if (missed < HANDSHAKE_MISSED_LIMIT) {
        return;
    }
    bridge_connection_set_connected(false);
}

/**
 * @brief Says whether it is time to send a request.
 *
 * @return false while the connection is made and the bridge has been heard from within the quiet
 *         limit; otherwise true for the first request and whenever the request interval has passed
 */
static bool request_due(void)
{
    if (bridge_connection_is_connected() && !time_interval_passed(HAL_GetTick(), last_heard_ms, HANDSHAKE_QUIET_LIMIT_MS)) {
        return false;
    }
    if (!request_sent) {
        return true;
    }
    return time_interval_passed(HAL_GetTick(), last_request_ms, HANDSHAKE_REQUEST_INTERVAL_MS);
}

void polling_send_when_due(char letter, const char *word)
{
    if (!request_due()) {
        return;
    }

    if (awaiting_ack) {
        note_missed();
    }

    awaiting_ack = true;
    request_sent = true;
    last_request_ms = HAL_GetTick();
    send_word(letter, word);
}
