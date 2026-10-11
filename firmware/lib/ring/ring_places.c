#include "ring_places.h"

/**
 * @brief Gives the place after this one in the ring.
 *
 * @param ring  The ring
 * @param place A place in the array
 * @return The next place, which is the first one after the last
 */
static uint32_t next_place(const ring_t *ring, uint32_t place)
{
    place++;
    if (place == ring->capacity) {
        return 0;
    }
    return place;
}

/**
 * @brief Gives the first free place in the ring, behind the newest item.
 *
 * @param ring The ring
 * @return The place the next item goes into
 */
static uint32_t free_place(const ring_t *ring)
{
    uint32_t place = ring->oldest + ring->count;

    if (place >= ring->capacity) {
        return place - ring->capacity;
    }
    return place;
}

bool ring_add(ring_t *ring, uint32_t *place)
{
    if (ring_is_full(ring)) {
        return false;
    }

    *place = free_place(ring);
    ring->count++;
    return true;
}

uint32_t ring_oldest_place(const ring_t *ring)
{
    return ring->oldest;
}

void ring_remove_oldest(ring_t *ring)
{
    if (ring_is_empty(ring)) {
        return;
    }

    ring->oldest = next_place(ring, ring->oldest);
    ring->count--;
}

bool ring_is_full(const ring_t *ring)
{
    return ring->count == ring->capacity;
}

bool ring_is_empty(const ring_t *ring)
{
    return ring->count == 0;
}
