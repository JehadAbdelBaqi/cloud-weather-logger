#include "message_helpers.h"

#include "stm32f4xx_hal.h"

#include "bridge_connection.h"
#include "config.h"
#include "line.h"
#include "link.h"
#include "protocol.h"

void send_word(char letter, const char *word)
{
    char line[LINE_BUFFER_SIZE];

    line_start(line, letter);
    line_add_field(line, word);
    link_send_line(line);
}

bool send_reply(const char *id, const char *result)
{
    char line[LINE_BUFFER_SIZE];

    if (!bridge_connection_is_connected()) {
        return false;
    }

    line_start(line, PROTOCOL_LETTER_REPLY);
    line_add_field(line, id);
    line_add_field(line, result);
    return link_send_line(line);
}

bool send_reading(const stored_reading_t *stored)
{
    char line[LINE_BUFFER_SIZE];

    line_start(line, PROTOCOL_LETTER_READING);
    line_add_number(line, stored->seq);
    line_add_number(line, stored->time);
    line_add_signed(line, stored->reading.temperature);
    line_add_number(line, stored->reading.pressure);
    line_add_number(line, stored->reading.humidity);

    return link_send_line(line);
}

uint32_t reading_time(void)
{
    return READING_TIME_START + HAL_GetTick() / 1000;
}
