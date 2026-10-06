#include "link_framing.h"

/**
 * @brief Closes the line being collected and gets ready for the next one.
 *
 * @param framing The line being collected
 * @return true if the closed line can be used, false if it was empty or too long
 */
static bool end_line(link_framing_t *framing)
{
    bool usable = !framing->too_long && framing->length > 0;

    framing->line[framing->length] = '\0';
    framing->length = 0;
    framing->too_long = false;
    return usable;
}

bool link_framing_take_byte(link_framing_t *framing, uint8_t byte)
{
    if (byte == '\r') {
        return false;
    }
    if (byte == '\n') {
        return end_line(framing);
    }
    if (framing->length >= LINK_LINE_MAX_LEN) {
        framing->too_long = true;
        return false;
    }

    framing->line[framing->length] = (char)byte;
    framing->length++;
    return false;
}
