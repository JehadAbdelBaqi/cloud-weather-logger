#include "link.h"

#include <string.h>


static uart_t link_uart;

// The line being collected from the bridge
static char rx_line[LINK_LINE_MAX_LEN + 1];  // room for the ending '\0'
static size_t rx_len = 0;
static bool rx_too_long = false;

/**
 * @brief Closes the line being collected and gets ready for the next one.
 *
 * @return true if the closed line can be used, false if it was empty or too long
 */
static bool end_line(void)
{
    bool usable = !rx_too_long && rx_len > 0;

    rx_line[rx_len] = '\0';
    rx_len = 0;
    rx_too_long = false;
    return usable;
}

/**
 * @brief Adds one received byte to the line being collected.
 *
 * @param byte The byte
 * @return true if this byte completed a usable line, now in rx_line
 */
static bool take_byte(uint8_t byte)
{
    if (byte == '\r') {
        return false;
    }
    if (byte == '\n') {
        return end_line();
    }
    if (rx_len >= LINK_LINE_MAX_LEN) {
        rx_too_long = true;
        return false;
    }

    rx_line[rx_len] = (char)byte;
    rx_len++;
    return false;
}

bool link_init(const uart_config_t *config)
{
    return uart_init(&link_uart, config);
}

bool link_send_line(const char *line)
{
    if (!uart_write(&link_uart, (const uint8_t *)line, strlen(line))) {
        return false;
    }
    return uart_write(&link_uart, (const uint8_t *)"\n", 1);
}

const char *link_read_line(void)
{
    uint8_t byte;

    while (uart_read_byte(&link_uart, &byte)) {
        if (take_byte(byte)) {
            return rx_line;
        }
    }
    return NULL;
}
