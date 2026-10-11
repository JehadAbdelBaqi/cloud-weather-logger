#include "link.h"

#include <string.h>


static uart_t link_uart;

// The bytes that have arrived from the bridge and not been read yet: put in by the interrupt,
// taken out by link_read_line()
static link_buffer_t rx_buffer;

// The line being collected from the bridge
static link_framing_t rx_framing;

bool link_init(const uart_config_t *config)
{
    if (!uart_init(&link_uart, config)) {
        return false;
    }
    return uart_enable_receive_interrupt(&link_uart);
}

void link_byte_arrived(void)
{
    uint8_t byte;

    if (!uart_take_received_byte(&link_uart, &byte)) {
        return;
    }
    link_buffer_put(&rx_buffer, byte);
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

    while (link_buffer_take(&rx_buffer, &byte)) {
        if (link_framing_take_byte(&rx_framing, byte)) {
            return rx_framing.line;
        }
    }
    return NULL;
}
