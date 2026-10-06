#ifndef TIME_INTERVAL_H
#define TIME_INTERVAL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Says whether an interval has passed since an earlier moment.
 *
 * The times are readings of a millisecond counter, such as HAL_GetTick(). The answer stays right
 * when the counter has wrapped round to 0 in between.
 *
 * @param now_ms      The counter now
 * @param since_ms    The counter at the earlier moment
 * @param interval_ms The interval
 * @return true if at least interval_ms have passed
 */
bool time_interval_passed(uint32_t now_ms, uint32_t since_ms, uint32_t interval_ms);

#endif
