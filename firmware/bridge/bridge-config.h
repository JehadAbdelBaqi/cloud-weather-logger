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

#endif
