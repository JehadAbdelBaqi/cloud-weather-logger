#include "link.h"

#include <string.h>


static uart_t link_uart;

// The line being collected from the bridge
static link_framing_t rx_framing;

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
        if (link_framing_take_byte(&rx_framing, byte)) {
            return rx_framing.line;
        }
    }
    return NULL;
}
