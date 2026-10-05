#include "reading.h"

#include <stddef.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "config.h"
#include "handshake.h"
#include "link.h"

/**
 * @brief Adds text to the end of a line. Adds nothing if there is not room for all of it.
 *
 * @param line The line so far, ending in '\0', in a buffer of LINK_LINE_MAX_LEN + 1 characters
 * @param text The text to add
 */
static void append_text(char *line, const char *text)
{
    size_t used = strlen(line);
    size_t length = strlen(text);

    if (used + length > LINK_LINE_MAX_LEN) {
        return;
    }
    memcpy(&line[used], text, length + 1);
}

/**
 * @brief Adds a whole number, in decimal, to the end of a line.
 *
 * @param line  The line so far
 * @param value The number
 */
static void append_number(char *line, uint32_t value)
{
    char text[11];  // 10 digits at most, then the ending '\0'
    char *digit = &text[sizeof(text) - 1];

    *digit = '\0';
    do {
        digit--;
        *digit = (char)('0' + value % 10);
        value /= 10;
    } while (value > 0);

    append_text(line, digit);
}

/**
 * @brief Adds a field to the end of a line: a comma, then a whole number.
 *
 * @param line  The line so far
 * @param value The number
 */
static void append_field(char *line, uint32_t value)
{
    append_text(line, ",");
    append_number(line, value);
}

/**
 * @brief Adds a field to the end of a line: a comma, then a whole number that may be negative.
 *
 * @param line  The line so far
 * @param value The number
 */
static void append_signed_field(char *line, int32_t value)
{
    uint32_t size = (uint32_t)value;

    append_text(line, ",");
    if (value < 0) {
        append_text(line, "-");
        size = 0u - size;
    }
    append_number(line, size);
}

/**
 * @brief Gives the time to put on a reading taken now.
 *
 * A stand-in until the RTC is set up: a fixed starting time plus the seconds since power-on.
 *
 * @return Unix time in seconds
 */
static uint32_t reading_time(void)
{
    return READING_TIME_START + HAL_GetTick() / 1000;
}

bool reading_send(const reading_t *reading)
{
    static uint32_t seq = 0;
    char line[LINK_LINE_MAX_LEN + 1] = "W";

    if (!handshake_connected()) {
        return false;
    }
    seq++;

    append_field(line, seq);
    append_field(line, reading_time());
    append_signed_field(line, reading->temperature);
    append_field(line, reading->pressure);
    append_field(line, reading->humidity);

    return link_send_line(line);
}
