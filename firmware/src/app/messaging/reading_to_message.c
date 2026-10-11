#include "reading_to_message.h"

#include <stddef.h>
#include <stdint.h>

#include "message_fields.h"
#include "protocol.h"

/**
 * @brief Turns the values every reading sends into text: its time, temperature, pressure and
 *        humidity, in that order.
 *
 * The one place the values are listed: a reading message (W) and the reply to a read command (R)
 * both take them from here, so the two have the same values in the same order.
 *
 * Every value is handled as a signed number, so the time is right until 19 January 2038, when
 * Unix time passes the largest signed 32-bit number.
 *
 * @param time    The time the reading was taken, as Unix time in seconds
 * @param reading The reading
 * @param texts   Filled in with the texts: READING_VALUE_COUNT buffers of FORMAT_SIGNED_SIZE
 *                characters
 */
void reading_values_to_text(uint32_t time, const reading_t *reading, char texts[][FORMAT_SIGNED_SIZE])
{
    const int32_t numbers[READING_VALUE_COUNT] = {
        (int32_t)time,
        reading->temperature,
        (int32_t)reading->pressure,
        (int32_t)reading->humidity,
    };

    numbers_to_text(numbers, READING_VALUE_COUNT, texts);
}

/**
 * @brief Fills in a reading message from a stored reading: its sequence number, then its values,
 *        each turned into text.
 *
 * @param stored  The reading, as taken from the store
 * @param message The message to fill in: the caller's own copy of reading_message
 * @param texts   Buffers for the texts, one of FORMAT_SIGNED_SIZE characters for each field.
 *                The message's fields point into them, so they have to last as long as the
 *                message is used
 */
void reading_to_message(const stored_reading_t *stored, message_t *message, char texts[][FORMAT_SIGNED_SIZE])
{
    const char *values[PROTOCOL_FIELDS_READING];

    format_signed((int32_t)stored->seq, texts[0]);
    reading_values_to_text(stored->time, &stored->reading, &texts[1]);

    for (size_t i = 0; i < PROTOCOL_FIELDS_READING; i++) {
        values[i] = texts[i];
    }
    fill_message(message, values);
}
