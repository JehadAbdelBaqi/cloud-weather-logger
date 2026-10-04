#include "debug.h"

#include <string.h>

#include "board.h"
#include "uart.h"

static uart_t debug_uart;

bool debug_init(void)
{
    const uart_config_t config = {
        .instance = DEBUG_UART,
        .port = DEBUG_PORT,
        .tx_pin = DEBUG_TX_PIN,
        .rx_pin = 0,  // output only
        .pin_af = DEBUG_PIN_AF,
        .baud = DEBUG_BAUD,
    };
    return uart_init(&debug_uart, &config);
}

void debug_print(const char *text)
{
    uart_write(&debug_uart, (const uint8_t *)text, strlen(text));
}

void debug_print_hex(uint8_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    const char text[] = { '0', 'x', digits[value >> 4], digits[value & 0x0F], '\0' };

    debug_print(text);
}
