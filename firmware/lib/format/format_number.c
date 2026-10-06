#include "format_number.h"

#include <stddef.h>
#include <string.h>

void format_number(uint32_t value, char *text)
{
    char digits[FORMAT_NUMBER_SIZE];
    char *digit = &digits[sizeof(digits) - 1];

    // Digits come out lowest first, so they are written from the end backwards
    *digit = '\0';
    do {
        digit--;
        *digit = (char)('0' + value % 10);
        value /= 10;
    } while (value > 0);

    memcpy(text, digit, (size_t)(&digits[sizeof(digits)] - digit));
}

void format_signed(int32_t value, char *text)
{
    if (value >= 0) {
        format_number((uint32_t)value, text);
        return;
    }

    text[0] = '-';
    format_number(0u - (uint32_t)value, &text[1]);
}

void format_hex_byte(uint8_t value, char *text)
{
    static const char digits[] = "0123456789ABCDEF";

    text[0] = '0';
    text[1] = 'x';
    text[2] = digits[value >> 4];
    text[3] = digits[value & 0x0F];
    text[4] = '\0';
}
