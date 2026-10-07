#ifndef BRIDGE_CONFIG_H
#define BRIDGE_CONFIG_H

// The weather logger's config for the Wi-Fi bridge's firmware.

#include "../include/protocol.h"

// The UART link to the Nucleo. The Nucleo's side has to use the same values.
#define UART_BAUD    115200  // bits per second
#define LINE_MAX_LEN 127     // longest line in either direction, in characters, not counting the '\n'

// Uplink: a line's first letter picks its topic. The letters are the Nucleo's own, from protocol.h
#define UPLINK_ROUTES \
    { PROTOCOL_LETTER_READING, "weather/nucleo-01/readings" }, \
    { PROTOCOL_LETTER_REPLY,   "weather/nucleo-01/replies" }

// Downlink: messages on these topics are written to the Nucleo
#define DOWNLINK_TOPICS \
    "weather/nucleo-01/acks", \
    "weather/nucleo-01/commands"

// Logs every line in both directions. 1 on, 0 off
#define LOG_LINES 1

// Handshake with the Nucleo ("H,request" / "H,ack")
#define HANDSHAKE_REQUEST_INTERVAL_MS 2000  // how often the request is repeated while it goes unanswered
#define HANDSHAKE_QUIET_LIMIT_MS      3000  // once connected: how long with no line from the Nucleo before asking
#define HANDSHAKE_MISSED_LIMIT        5     // unanswered requests in a row before the connection counts as lost

#endif
