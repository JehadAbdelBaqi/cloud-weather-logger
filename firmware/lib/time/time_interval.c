#include "time_interval.h"

bool time_interval_passed(uint32_t now_ms, uint32_t since_ms, uint32_t interval_ms)
{
    return now_ms - since_ms >= interval_ms;
}
