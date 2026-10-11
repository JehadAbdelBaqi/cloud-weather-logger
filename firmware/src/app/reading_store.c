#include "reading_store.h"

#include "config.h"

// The readings, in a ring: the oldest is at `oldest`, and `count` of them follow it, going back
// to the start of the array after its last place
static stored_reading_t readings[READING_STORE_CAPACITY];
static uint32_t oldest = 0;
static uint32_t count = 0;

// The sequence number given to the last reading stored
static uint32_t last_seq = 0;

/**
 * @brief Gives the place after this one in the ring.
 *
 * @param place A place in the array
 * @return The next place, which is the first one after the last
 */
static uint32_t next_place(uint32_t place)
{
    place++;
    if (place == READING_STORE_CAPACITY) {
        return 0;
    }
    return place;
}

/**
 * @brief Gives the first free place in the ring, behind the newest reading.
 *
 * @return The place the next reading goes into
 */
static uint32_t free_place(void)
{
    uint32_t place = oldest + count;

    if (place >= READING_STORE_CAPACITY) {
        return place - READING_STORE_CAPACITY;
    }
    return place;
}

bool reading_store_put(const reading_t *reading, uint32_t time)
{
    if (reading_store_is_full()) {
        return false;
    }

    stored_reading_t *place = &readings[free_place()];

    last_seq++;
    place->seq = last_seq;
    place->time = time;
    place->reading = *reading;

    count++;
    return true;
}

bool reading_store_oldest(stored_reading_t *stored)
{
    if (count == 0) {
        return false;
    }

    *stored = readings[oldest];
    return true;
}

void reading_store_remove_oldest(void)
{
    if (count == 0) {
        return;
    }

    oldest = next_place(oldest);
    count--;
}

bool reading_store_remove(uint32_t seq)
{
    if (count == 0) {
        return false;
    }
    if (readings[oldest].seq != seq) {
        return false;  // not the oldest reading: nothing is removed
    }

    reading_store_remove_oldest();
    return true;
}

bool reading_store_is_full(void)
{
    return count == READING_STORE_CAPACITY;
}

bool reading_store_is_empty(void)
{
    return count == 0;
}
