#include "link_buffer.h"

/**
 * @brief Gives the place after a given one, going back to the start after the last.
 *
 * @param place A place in the buffer
 * @return The next place
 */
static uint32_t next_place(uint32_t place)
{
    place++;
    if (place == LINK_BUFFER_SIZE) {
        return 0;
    }
    return place;
}

bool link_buffer_put(link_buffer_t *buffer, uint8_t byte)
{
    uint32_t next = next_place(buffer->put_place);

    // One place is always left empty, so that full and empty can be told apart
    if (next == buffer->take_place) {
        return false;
    }
    buffer->bytes[buffer->put_place] = byte;
    buffer->put_place = next;
    return true;
}

bool link_buffer_take(link_buffer_t *buffer, uint8_t *byte)
{
    if (buffer->take_place == buffer->put_place) {
        return false;
    }
    *byte = buffer->bytes[buffer->take_place];
    buffer->take_place = next_place(buffer->take_place);
    return true;
}
