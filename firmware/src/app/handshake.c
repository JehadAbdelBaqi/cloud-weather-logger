#include "handshake.h"

#include <stdint.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "config.h"
#include "debug.h"
#include "link.h"

#define HANDSHAKE_LETTER 'H'
#define REQUEST_LINE     "H,request"
#define ACK_LINE         "H,ack"

static bool connected = false;
static bool awaiting_ack = false;    // a request has been sent and not answered yet
static int missed = 0;               // requests in a row that went unanswered
static bool request_sent = false;    // false until the first request has gone out
static uint32_t last_request_ms = 0;
static uint32_t last_heard_ms = 0;   // when the last line arrived from the bridge

/**
 * @brief Records whether the connection to the bridge is made, and says so on the debug output.
 *
 * Writes only when the state changes.
 *
 * @param now_connected true once the bridge has answered, false when it is lost
 */
static void set_connected(bool now_connected)
{
    bool changed = (now_connected != connected);

    connected = now_connected;

    if (!changed) {
        return;
    }
    if (now_connected) {
        debug_print("bridge connected\r\n");
        return;
    }
    debug_print("bridge lost: waiting for the bridge...\r\n");
}

/**
 * @brief Notes that a line has arrived from the bridge: it is still there, and nothing is missed.
 *
 * Does not make the connection; only a handshake line does that.
 */
static void note_heard(void)
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
    if (!connected) {
        return;  // still trying to make the connection: nothing to lose
    }

    missed++;
    if (missed < HANDSHAKE_MISSED_LIMIT) {
        return;
    }
    set_connected(false);
}

/**
 * @brief Says whether it is time to send a request.
 *
 * @return false while the connection is made and the bridge has been heard from within the quiet
 *         limit; otherwise true for the first request and whenever the request interval has passed
 */
static bool request_due(void)
{
    if (connected && HAL_GetTick() - last_heard_ms < HANDSHAKE_QUIET_LIMIT_MS) {
        return false;
    }
    if (!request_sent) {
        return true;
    }
    return HAL_GetTick() - last_request_ms >= HANDSHAKE_REQUEST_INTERVAL_MS;
}

void handshake_init(void)
{
    debug_print("waiting for the bridge...\r\n");
}

void handshake_poll(void)
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
    link_send_line(REQUEST_LINE);
}

bool handshake_handle_line(const char *line)
{
    note_heard();

    if (line[0] != HANDSHAKE_LETTER) {
        return false;
    }

    if (strcmp(line, REQUEST_LINE) == 0) {
        link_send_line(ACK_LINE);
        set_connected(true);
        return true;
    }

    if (strcmp(line, ACK_LINE) == 0) {
        set_connected(true);
        return true;
    }

    debug_print("line ignored: not a handshake line\r\n");
    return true;
}

bool handshake_connected(void)
{
    return connected;
}
