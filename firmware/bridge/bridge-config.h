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
#define HANDSHAKE_RETRY_INTERVAL_MS 1000   // while not connected: how often the request is repeated
#define HANDSHAKE_CHECK_INTERVAL_MS 20000  // once connected: how often the connection is checked
#define HANDSHAKE_MISSED_LIMIT      2      // unanswered checks in a row before it counts as lost

#endif
