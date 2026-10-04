#include "link.h"

#include <string.h>

#include "board.h"
#include "uart.h"

static uart_t link_uart;

bool link_init(void)
{
    const uart_config_t config = {
        .instance = LINK_UART,
        .port = LINK_PORT,
        .tx_pin = LINK_TX_PIN,
        .rx_pin = LINK_RX_PIN,
        .pin_af = LINK_PIN_AF,
        .baud = LINK_BAUD,
    };
    return uart_init(&link_uart, &config);
}

bool link_send_line(const char *line)
{
    if (!uart_write(&link_uart, (const uint8_t *)line, strlen(line))) {
        return false;
    }
    return uart_write(&link_uart, (const uint8_t *)"\n", 1);
}
