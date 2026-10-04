#ifndef CONFIG_H
#define CONFIG_H

// The Nucleo firmware's tunable values. All times are in milliseconds.

// Handshake with the bridge ("H,request" / "H,ack")
#define HANDSHAKE_REQUEST_INTERVAL_MS 2000  // how often the request is repeated while it goes unanswered
#define HANDSHAKE_QUIET_LIMIT_MS      3000  // once connected: how long with no line from the bridge before asking
#define HANDSHAKE_MISSED_LIMIT        5     // unanswered requests in a row before the connection counts as lost

// User LED: on / off time while the connection to the bridge is being made, and once it is made
#define LED_CONNECTING_BLINK_MS 100
#define LED_CONNECTED_BLINK_MS  500

#endif
