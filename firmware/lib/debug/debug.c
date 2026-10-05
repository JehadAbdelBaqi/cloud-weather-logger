#include "debug.h"

#include <string.h>

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
    static const char digits[] = "0123456789ABCDEF";
    const char text[] = { '0', 'x', digits[value >> 4], digits[value & 0x0F], '\0' };

    debug_print(text);
}

void debug_print_number(uint32_t value)
{
    char text[11];  // 10 digits at most, then the ending '\0'
    char *digit = &text[sizeof(text) - 1];

    *digit = '\0';
    do {
        digit--;
        *digit = (char)('0' + value % 10);
        value /= 10;
    } while (value > 0);

    debug_print(digit);
}

void debug_print_signed(int32_t value)
{
    if (value >= 0) {
        debug_print_number((uint32_t)value);
        return;
    }

    debug_print("-");
    debug_print_number(0u - (uint32_t)value);
}
