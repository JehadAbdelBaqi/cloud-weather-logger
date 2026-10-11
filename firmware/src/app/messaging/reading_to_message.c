#include "reading_to_message.h"

#include <stdint.h>

#include "message_fields.h"
#include "protocol.h"

/**
 * @brief Fills in a reading message from a stored reading: its five values, in the order of
 *        the message's fields, each turned into text.
 *
 * Every value is handled as a signed number, so the time is right until 19 January 2038, when
 * Unix time passes the largest signed 32-bit number.
 *
 * @param stored  The reading, as taken from the store
 * @param message The message to fill in: the caller's own copy of reading_message
 * @param texts   Buffers for the texts, one of FORMAT_SIGNED_SIZE characters for each value.
 *                The message's fields point into them, so they have to last as long as the
 *                message is used
 */
void reading_to_message(const stored_reading_t *stored, message_t *message, char texts[][FORMAT_SIGNED_SIZE])
{
    const int32_t numbers[PROTOCOL_FIELDS_READING] = {
        (int32_t)stored->seq,
        (int32_t)stored->time,
        stored->reading.temperature,
        (int32_t)stored->reading.pressure,
        (int32_t)stored->reading.humidity,
    };

    fill_message_fields_from_numbers(message, numbers, texts);
}
