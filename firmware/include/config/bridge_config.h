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

// How the bridge cuts the bytes from the Nucleo into lines: the characters that end a line, and
// the ones it leaves out. The same rule the Nucleo's own link library works by. The bridge ends
// the lines it sends with the first of the line-end characters
#define LINE_END_CHARACTERS     "\n"
#define LINE_SKIPPED_CHARACTERS "\r"

// Uplink: a line's first letter picks its topic. The letters are from protocol.h, the topics from
// topic_routes.h
#define UPLINK_ROUTES \
    { PROTOCOL_LETTER_READING, TOPIC_READINGS }, \
    { PROTOCOL_LETTER_REPLY,   TOPIC_REPLIES }

// Downlink: messages on these topics are written to the Nucleo
#define DOWNLINK_TOPICS \
    TOPIC_ACKS, \
    TOPIC_COMMANDS

// The letter of the Nucleo's handshake line, which the bridge answers itself and never publishes
#define HANDSHAKE_LETTER PROTOCOL_LETTER_HANDSHAKE

// The bridge's answer to a line from the Nucleo (the "L" line of protocol.h). The bridge fills in
// two things: the start of the line it answers, and its status. The start is the line's first
// ANSWER_FIELD_COUNT fields, counting its letter as one: "W,5" of "W,5,2350"
#define ANSWER_FORMAT      "L,%s,%s"
#define ANSWER_FIELD_COUNT 2
#define FIELD_SEPARATOR    PROTOCOL_FIELD_SEPARATOR

// The text the bridge gives for each of its statuses, in an answer: the words of protocol.h
#define STATUS_TEXT_OK             PROTOCOL_REPLY_OK
#define STATUS_TEXT_CON_ERR_WIFI   PROTOCOL_RESPONSE_NO_WIFI
#define STATUS_TEXT_CON_ERR_BROKER PROTOCOL_RESPONSE_NO_BROKER
#define STATUS_TEXT_ERR_BROKER     PROTOCOL_RESPONSE_BROKER_ERR
#define STATUS_TEXT_ANS_ERR_LONG   PROTOCOL_RESPONSE_TOO_LONG

// Logs every line in both directions. 1 on, 0 off
#define LOG_LINES 1

// How long with no line from the Nucleo before the bridge counts it as gone. The Nucleo sends
// something at least once every quiet limit, so this is that limit and a margin after it
#if MCU_QUIET_BUFFER_MS < 1000
#error "MCU_QUIET_BUFFER_MS in config.h must be at least 1000"
#endif
#define MCU_QUIET_LIMIT_MS (ACK_QUIET_LIMIT_MS + MCU_QUIET_BUFFER_MS)

// How long the bridge waits for the broker to confirm a line before it answers the Nucleo that
// the line has not gone through. Worked out from the Nucleo's quiet limit, so it is always the
// shorter of the two
#if BRIDGE_ANSWER_BUFFER_MS < 1000
#error "BRIDGE_ANSWER_BUFFER_MS in config.h must be at least 1000"
#endif
#if BRIDGE_ANSWER_BUFFER_MS >= ACK_QUIET_LIMIT_MS
#error "BRIDGE_ANSWER_BUFFER_MS in config.h must be less than ACK_QUIET_LIMIT_MS"
#endif
#define BROKER_CONFIRM_LIMIT_MS (ACK_QUIET_LIMIT_MS - BRIDGE_ANSWER_BUFFER_MS)

#endif
