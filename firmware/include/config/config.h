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

// How often the sensor is read, until a "rate" command changes it
#define SENSOR_READ_INTERVAL_MS 2000

// The shortest and longest interval a "rate" command may set, in seconds
#define SENSOR_READ_INTERVAL_MIN_S 1
#define SENSOR_READ_INTERVAL_MAX_S 3600

// Stand-in for the time of a reading until the RTC is set up: the time starts here at every
// power-on and counts up. Unix time in seconds: 5 October 2026, 00:00 UK time
#define READING_TIME_START 1791154800u

#endif
