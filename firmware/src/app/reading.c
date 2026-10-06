#include "reading.h"

#include <stddef.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "config.h"
#include "format_number.h"
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
 * @brief Adds a field to the end of a line: a comma, then a whole number.
 *
 * @param line  The line so far
 * @param value The number
 */
static void append_field(char *line, uint32_t value)
{
    char text[FORMAT_NUMBER_SIZE];

    format_number(value, text);
    append_text(line, ",");
    append_text(line, text);
}

/**
 * @brief Adds a field to the end of a line: a comma, then a whole number that may be negative.
 *
 * @param line  The line so far
 * @param value The number
 */
static void append_signed_field(char *line, int32_t value)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(value, text);
    append_text(line, ",");
    append_text(line, text);
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
