#include "message_helpers.h"

#include "stm32f4xx_hal.h"

#include "config.h"
#include "line.h"
#include "link.h"

void send_word(char letter, const char *word)
{
    char line[LINE_BUFFER_SIZE];

    line_start(line, letter);
    line_add_field(line, word);
    link_send_line(line);
}

uint32_t reading_time(void)
{
    return READING_TIME_START + HAL_GetTick() / 1000;
}
