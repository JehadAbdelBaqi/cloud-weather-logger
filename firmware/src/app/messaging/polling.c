#include "polling.h"

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

#include "bridge_connection.h"
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
 *
 * @param missed_limit How many requests in a row may go unanswered before the connection is lost
 */
static void note_missed(int missed_limit)
{
    if (!bridge_connection_is_connected()) {
        return;  // still trying to make the connection: nothing to lose
    }

    missed++;
    if (missed < missed_limit) {
        return;
    }
    bridge_connection_set_connected(false);
}

/**
 * @brief Says whether it is time to send a request.
 *
 * @param quiet_limit_ms      How long nothing may be heard from the bridge before a request is due
 * @param request_interval_ms The time between one request and the next
 * @return false while the connection is made and the bridge has been heard from within the quiet
 *         limit; otherwise true for the first request and whenever the request interval has passed
 */
static bool request_due(uint32_t quiet_limit_ms, uint32_t request_interval_ms)
{
    if (bridge_connection_is_connected() && !time_interval_passed(HAL_GetTick(), last_heard_ms, quiet_limit_ms)) {
        return false;
    }
    if (!request_sent) {
        return true;
    }
    return time_interval_passed(HAL_GetTick(), last_request_ms, request_interval_ms);
}

void polling_send_when_due(char letter, const char *word, uint32_t quiet_limit_ms, uint32_t request_interval_ms, int missed_limit)
{
    if (!request_due(quiet_limit_ms, request_interval_ms)) {
        return;
    }

    if (awaiting_ack) {
        note_missed(missed_limit);
    }

    awaiting_ack = true;
    request_sent = true;
    last_request_ms = HAL_GetTick();
    send_word(letter, word);
}
