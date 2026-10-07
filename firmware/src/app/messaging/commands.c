#include "commands.h"

#include <stddef.h>
#include <stdint.h>

#include "line.h"
#include "message_helpers.h"
#include "protocol.h"
#include "sensor.h"

// What "help" replies with: the commands, with a space between them
#define HELP_TEXT PROTOCOL_COMMAND_HELP " " PROTOCOL_COMMAND_READ " " PROTOCOL_COMMAND_RATE " <s>"

/** A function that carries out one command and sends its reply. */
typedef void (*command_function_t)(const char *id, const char *value);

/** One row of the command table: this word runs this function. */
typedef struct {
    const char *word;
    command_function_t function;
} command_t;

/**
 * @brief "help": replies with the list of commands.
 *
 * @param id    The ID of the command
 * @param value Not used
 */
static void command_help(const char *id, const char *value)
{
    (void)value;
    send_reply(id, HELP_TEXT);
}

/**
 * @brief "read": takes a reading now and sends it as a normal reading line, then replies.
 *
 * @param id    The ID of the command
 * @param value Not used
 */
static void command_read(const char *id, const char *value)
{
    (void)value;

    if (!sensor_read_now()) {
        send_reply(id, PROTOCOL_REPLY_ERR " no reading");
        return;
    }
    send_reply(id, PROTOCOL_REPLY_OK);
}

/**
 * @brief "rate <s>": sets how often readings are taken, then replies.
 *
 * @param id    The ID of the command
 * @param value The number of seconds as text, or NULL if none was given
 */
static void command_rate(const char *id, const char *value)
{
    uint32_t seconds = 0;

    if (value == NULL) {
        send_reply(id, PROTOCOL_REPLY_ERR " rate needs a number of seconds");
        return;
    }
    if (!line_read_number(value, &seconds)) {
        send_reply(id, PROTOCOL_REPLY_ERR " rate needs a number of seconds");
        return;
    }
    if (!sensor_set_interval(seconds)) {
        send_reply(id, PROTOCOL_REPLY_ERR " rate out of range");
        return;
    }
    send_reply(id, PROTOCOL_REPLY_OK);
}

// The commands, and the function that carries out each
static const command_t commands[] = {
    { PROTOCOL_COMMAND_HELP, command_help },
    { PROTOCOL_COMMAND_READ, command_read },
    { PROTOCOL_COMMAND_RATE, command_rate },
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

void commands_run(const char *id, const char *command)
{
    const char *value = NULL;

    for (size_t i = 0; i < COMMAND_COUNT; i++) {
        if (line_take_word(command, commands[i].word, &value)) {
            commands[i].function(id, value);
            return;
        }
    }

    send_reply(id, PROTOCOL_REPLY_ERR " unknown command");
}
