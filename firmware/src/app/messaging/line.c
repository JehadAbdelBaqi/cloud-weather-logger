#include "line.h"

#include <string.h>

#include "format_number.h"
#include "protocol.h"

void line_start(char *line, char letter)
{
    line[0] = letter;
    line[1] = '\0';
}

bool line_add_field(char *line, const char *text)
{
    size_t used = strlen(line);
    size_t length = strlen(text);

    if (used + 1 + length > LINK_LINE_MAX_LEN) {
        return false;
    }

    line[used] = PROTOCOL_FIELD_SEPARATOR;
    memcpy(&line[used + 1], text, length + 1);
    return true;
}

bool line_add_number(char *line, uint32_t value)
{
    char text[FORMAT_NUMBER_SIZE];

    format_number(value, text);
    return line_add_field(line, text);
}

bool line_add_signed(char *line, int32_t value)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(value, text);
    return line_add_field(line, text);
}

const char *line_field(const char *line, size_t index, size_t *length)
{
    const char separator[] = { PROTOCOL_FIELD_SEPARATOR, '\0' };
    const char *field = line;

    // Step past the letter, then past each field before the one wanted
    for (size_t passed = 0; passed <= index; passed++) {
        field = strchr(field, PROTOCOL_FIELD_SEPARATOR);
        if (field == NULL) {
            return NULL;
        }
        field++;
    }

    *length = strcspn(field, separator);
    return field;
}

bool message_word_is(const char *line, const char *word)
{
    size_t length = 0;
    const char *field = line_field(line, 0, &length);

    if (field == NULL) {
        return false;
    }
    return strcmp(field, word) == 0;
}

bool line_copy_field(const char *line, size_t index, char *text, size_t size)
{
    size_t length = 0;
    const char *field = line_field(line, index, &length);

    if (field == NULL) {
        return false;
    }
    if (length >= size) {
        return false;
    }

    memcpy(text, field, length);
    text[length] = '\0';
    return true;
}

bool line_take_word(const char *text, const char *word, const char **rest)
{
    size_t length = strlen(word);

    if (strncmp(text, word, length) != 0) {
        return false;
    }
    if (text[length] == '\0') {
        *rest = NULL;
        return true;
    }
    if (text[length] != ' ') {
        return false;
    }

    *rest = &text[length + 1];
    return true;
}

bool line_read_number(const char *text, uint32_t *value)
{
    uint64_t total = 0;

    if (*text == '\0') {
        return false;
    }

    for (; *text != '\0'; text++) {
        if (*text < '0' || *text > '9') {
            return false;
        }
        total = total * 10 + (uint64_t)(*text - '0');
        if (total > UINT32_MAX) {
            return false;
        }
    }

    *value = (uint32_t)total;
    return true;
}
