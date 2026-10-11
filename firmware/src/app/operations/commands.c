#include "commands.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "format_number.h"
#include "message_build.h"
#include "protocol.h"
#include "reading.h"
#include "reading_to_message.h"
#include "routing.h"
#include "sensor.h"

// The results for a command that cannot be carried out
#define RESULT_NO_COMMAND      PROTOCOL_REPLY_ERR " no command"
#define RESULT_UNKNOWN_COMMAND PROTOCOL_REPLY_ERR " unknown command"
#define RESULT_NO_READING      PROTOCOL_REPLY_ERR " no reading"
#define RESULT_RATE_NO_NUMBER  PROTOCOL_REPLY_ERR " rate needs a number of seconds"
#define RESULT_RATE_RANGE      PROTOCOL_REPLY_ERR " rate out of range"

// A number from protocol.h as text, so that the help result takes each command's number from there
#define AS_TEXT_STEP(value) #value
#define AS_TEXT(value)      AS_TEXT_STEP(value)

// The result of the help command: each command's number and its name
#define RESULT_HELP AS_TEXT(PROTOCOL_COMMAND_HELP) "=help " \
                    AS_TEXT(PROTOCOL_COMMAND_READ) "=read " \
                    AS_TEXT(PROTOCOL_COMMAND_RATE) "=rate"

/**
 * @brief The help command: gives the list of commands, each with its number.
 *
 * @param state  The program's running state: not used
 * @param value  The value that came with the command: not used
 * @param result Filled in with the list
 */
void command_help(app_state_t *state, const char *value, char *result)
{
    (void)state;
    (void)value;

    strcpy(result, RESULT_HELP);
}

/**
 * @brief The read command: takes a fresh reading from the sensor now and gives it as the result,
 *        so the reading goes up as the reply: R,<id>,<time>,<temperature>,<pressure>,<humidity>.
 *
 * The reading is not put in the reading store and has no sequence number: the command's ID names
 * it. The timed readings carry on at their own times.
 *
 * @param state  The program's running state: its time is the time the reading is given
 * @param value  The value that came with the command: not used
 * @param result Filled in with the reading's four values, each as text with the field separator
 *               between them, or with an error if the sensor could not be read
 */
void command_read(app_state_t *state, const char *value, char *result)
{
    reading_t reading;
    char texts[READING_VALUE_COUNT][FORMAT_SIGNED_SIZE];

    (void)value;

    if (!sensor_read_now(&reading)) {
        strcpy(result, RESULT_NO_READING);
        return;
    }

    reading_values_to_text(READING_TIME_START + state->now_ms / 1000, &reading, texts);
    strcpy(result, texts[0]);
    for (size_t i = 1; i < READING_VALUE_COUNT; i++) {
        message_add_field(result, PROTOCOL_FIELD_SEPARATOR, texts[i]);
    }
}

/**
 * @brief The rate command: sets how often readings are taken.
 *
 * The interval lasts until the Nucleo restarts.
 *
 * @param state  The program's running state: the read interval of its sensor timer is set
 * @param value  The interval in whole seconds, as text, from SENSOR_READ_INTERVAL_MIN_S to
 *               SENSOR_READ_INTERVAL_MAX_S
 * @param result Filled in with ok, or with an error if the value is not a number or is outside
 *               those limits
 */
void command_rate(app_state_t *state, const char *value, char *result)
{
    uint32_t seconds = 0;

    if (!format_read_number(value, &seconds)) {
        strcpy(result, RESULT_RATE_NO_NUMBER);
        return;
    }
    if (seconds < SENSOR_READ_INTERVAL_MIN_S) {
        strcpy(result, RESULT_RATE_RANGE);
        return;
    }
    if (seconds > SENSOR_READ_INTERVAL_MAX_S) {
        strcpy(result, RESULT_RATE_RANGE);
        return;
    }

    state->sensor_timer.read_interval_ms = seconds * 1000;
    strcpy(result, PROTOCOL_REPLY_OK);
}

/**
 * @brief Runs a command: reads its number and calls the function that number leads to in the
 *        table of commands, which fills in the result.
 *
 * A command that is empty, is not a number, or has no function in the table gets a result
 * saying so.
 *
 * @param state   The program's running state, handed on to the command's function
 * @param command The command as it arrived: its number, as text, e.g. "3"
 * @param value   The value that came with the command, e.g. "10"; empty if there was none
 * @param result  Filled in with the result for the reply; REPLY_RESULT_SIZE characters
 */
void run_command(app_state_t *state, const char *command, const char *value, char *result)
{
    uint32_t number = 0;

    if (command[0] == '\0') {
        strcpy(result, RESULT_NO_COMMAND);
        return;
    }
    if (!format_read_number(command, &number)) {
        strcpy(result, RESULT_UNKNOWN_COMMAND);
        return;
    }
    if (!call_command_route(number, state, value, result)) {
        strcpy(result, RESULT_UNKNOWN_COMMAND);
    }
}
