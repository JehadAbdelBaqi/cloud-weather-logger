#include "sensor_timer.h"

#include "sensor.h"
#include "time_interval.h"

bool sensor_poll(sensor_timer_t *timer, uint32_t now_ms, reading_t *reading)
{
    if (!time_interval_passed(now_ms, timer->last_read_ms, timer->read_interval_ms)) {
        return false;
    }
    timer->last_read_ms = now_ms;

    return sensor_read_now(reading);
}
