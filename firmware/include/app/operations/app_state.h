#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "sensor_timer.h"
#include "states.h"

// What lasts from one pass of the main loop to the next. Declared once, in main.c, and handed to
// every function that needs it
typedef struct app_state {
    bool awaiting_handshake_ack;  // true while a handshake (H) is out and not answered yet
    bool awaiting_reading_ack;    // true while a reading (W) is out and not answered yet
    bool awaiting_reply_ack;      // true while a reply (R) is out and not answered yet
    uint32_t now_ms;              // the time now: the same for the whole of one pass
    uint32_t sent_ms;             // the time the last message went out
    state_key_t current_state;    // the state the program is in
    sensor_timer_t sensor_timer;  // the times the sensor is read by
} app_state_t;

#endif
