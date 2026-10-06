#include "debug.h"

#include <string.h>

#include "format_number.h"

static uart_t debug_uart;

bool debug_init(const uart_config_t *config)
{
    return uart_init(&debug_uart, config);
}

void debug_print(const char *text)
{
    uart_write(&debug_uart, (const uint8_t *)text, strlen(text));
}

void debug_print_hex(uint8_t value)
{
    char text[FORMAT_HEX_BYTE_SIZE];

    format_hex_byte(value, text);
    debug_print(text);
}

void debug_print_number(uint32_t value)
{
    char text[FORMAT_NUMBER_SIZE];

    format_number(value, text);
    debug_print(text);
}

void debug_print_signed(int32_t value)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(value, text);
    debug_print(text);
}
