#include "sensor_timer.h"

#include "stm32f4xx_hal.h"

#include "config.h"
#include "sensor.h"
#include "time_interval.h"

// How often sensor_poll() takes a reading, in milliseconds
static uint32_t read_interval_ms = SENSOR_READ_INTERVAL_MS;

bool sensor_poll(reading_t *reading)
{
    static uint32_t last_read_ms = 0;

    if (!time_interval_passed(HAL_GetTick(), last_read_ms, read_interval_ms)) {
        return false;
    }
    last_read_ms = HAL_GetTick();

    return sensor_read_now(reading);
}

bool sensor_set_interval(uint32_t seconds)
{
    if (seconds < SENSOR_READ_INTERVAL_MIN_S) {
        return false;
    }
    if (seconds > SENSOR_READ_INTERVAL_MAX_S) {
        return false;
    }

    read_interval_ms = seconds * 1000;
    return true;
}
