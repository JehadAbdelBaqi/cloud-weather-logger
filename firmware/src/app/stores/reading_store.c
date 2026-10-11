#include "reading_store.h"

#include "config.h"
#include "ring_places.h"

// The readings, in a ring: the ring says which place holds the oldest and which is free
static stored_reading_t readings[READING_STORE_CAPACITY];
static ring_t ring = { .capacity = READING_STORE_CAPACITY };

// The sequence number given to the last reading stored
static uint32_t last_seq = 0;

bool reading_store_put(const reading_t *reading, uint32_t time)
{
    uint32_t place;

    if (!ring_add(&ring, &place)) {
        return false;
    }

    last_seq++;
    readings[place].seq = last_seq;
    readings[place].time = time;
    readings[place].reading = *reading;
    return true;
}

bool reading_store_oldest(stored_reading_t *stored)
{
    if (ring_is_empty(&ring)) {
        return false;
    }

    *stored = readings[ring_oldest_place(&ring)];
    return true;
}

void reading_store_remove_oldest(void)
{
    ring_remove_oldest(&ring);
}

bool reading_store_remove(uint32_t seq)
{
    if (ring_is_empty(&ring)) {
        return false;
    }
    if (readings[ring_oldest_place(&ring)].seq != seq) {
        return false;  // not the oldest reading: nothing is removed
    }

    ring_remove_oldest(&ring);
    return true;
}

bool reading_store_is_full(void)
{
    return ring_is_full(&ring);
}

bool reading_store_is_empty(void)
{
    return ring_is_empty(&ring);
}
