#ifndef BRIDGE_CONFIG_H
#define BRIDGE_CONFIG_H

// The weather logger's config for the Wi-Fi bridge's firmware.

// Uplink: a line's first letter picks its topic
#define UPLINK_ROUTES \
    { 'W', "weather/nucleo-01/readings" }, \
    { 'R', "weather/nucleo-01/replies" }

// Downlink: messages on these topics are written to the Nucleo
#define DOWNLINK_TOPICS \
    "weather/nucleo-01/acks", \
    "weather/nucleo-01/commands"

// Logs every line in both directions. 1 on, 0 off
#define LOG_LINES 1

// Handshake with the Nucleo ("H,request" / "H,ack")
#define HANDSHAKE_REQUEST_INTERVAL_MS 2000  // how often the request is repeated while it goes unanswered
#define HANDSHAKE_QUIET_LIMIT_MS      5000  // once connected: how long with no line from the Nucleo before asking
#define HANDSHAKE_MISSED_LIMIT        5     // unanswered requests in a row before the connection counts as lost

#endif
