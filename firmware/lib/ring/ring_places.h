#ifndef RING_PLACES_H
#define RING_PLACES_H

#include <stdbool.h>
#include <stdint.h>

// The places of a ring: an array used so that the oldest item is taken out first, going back to
// the start of the array after its last place. This keeps count of the places only. The array
// and what is in it belong to the caller, which asks here which place to use

/** Where a ring's items are: the oldest is at `oldest`, and `count` of them follow it. */
typedef struct {
    uint32_t oldest;    // the place of the oldest item
    uint32_t count;     // how many items the ring holds
    uint32_t capacity;  // how many places the caller's array has
} ring_t;

/**
 * @brief Takes the first free place in the ring, behind the newest item, for a new item.
 *
 * @param ring  The ring: it then holds one more item
 * @param place Filled in with the place the caller puts the new item into; left as it was if the
 *              ring is full
 * @return true if a place was taken, false if the ring is full
 */
bool ring_add(ring_t *ring, uint32_t *place);

/**
 * @brief Gives the place of the oldest item in the ring.
 *
 * Only means something while the ring holds an item: check ring_is_empty() first.
 *
 * @param ring The ring
 * @return The place of the oldest item
 */
uint32_t ring_oldest_place(const ring_t *ring);

/**
 * @brief Removes the oldest item from the ring. Does nothing if the ring is empty.
 *
 * @param ring The ring: it then holds one item fewer, and the next one is the oldest
 */
void ring_remove_oldest(ring_t *ring);

/**
 * @brief Says whether the ring has no room for another item.
 *
 * @param ring The ring
 * @return true if the ring is full
 */
bool ring_is_full(const ring_t *ring);

/**
 * @brief Says whether the ring holds no items.
 *
 * @param ring The ring
 * @return true if the ring is empty
 */
bool ring_is_empty(const ring_t *ring);

#endif
