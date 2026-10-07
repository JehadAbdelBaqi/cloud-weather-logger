#ifndef BRIDGE_CONFIG_H
#define BRIDGE_CONFIG_H

// The weather logger's config for the Wi-Fi bridge's firmware.

#include "board_config.h"
#include "config.h"
#include "protocol.h"
#include "topic_routes.h"

// The UART link to the Nucleo: the same values the Nucleo is built with, from board_config.h
#define UART_BAUD    LINK_BAUD
#define LINE_MAX_LEN LINK_LINE_LIMIT

// Uplink: a line's first letter picks its topic. The letters are from protocol.h, the topics from
// topic_routes.h
#define UPLINK_ROUTES \
    { PROTOCOL_LETTER_READING, TOPIC_READINGS }, \
    { PROTOCOL_LETTER_REPLY,   TOPIC_REPLIES }

// Downlink: messages on these topics are written to the Nucleo
#define DOWNLINK_TOPICS \
    TOPIC_ACKS, \
    TOPIC_COMMANDS

// Logs every line in both directions. 1 on, 0 off
#define LOG_LINES 1

// Handshake with the Nucleo ("H,request" / "H,ack"): HANDSHAKE_REQUEST_INTERVAL_MS,
// HANDSHAKE_QUIET_LIMIT_MS and HANDSHAKE_MISSED_LIMIT are the Nucleo's own, from config.h

#endif
