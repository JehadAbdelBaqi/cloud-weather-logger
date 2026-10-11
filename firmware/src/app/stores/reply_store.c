#include "reply_store.h"

#include <stdint.h>
#include <string.h>

#include "config.h"
#include "ring_places.h"

// The replies, in a ring: the ring says which place holds the oldest and which is free
static stored_reply_t replies[REPLY_STORE_CAPACITY];
static ring_t ring = { .capacity = REPLY_STORE_CAPACITY };

bool reply_store_put(const char *id, const char *result)
{
    uint32_t place;

    if (strlen(id) >= REPLY_ID_SIZE) {
        return false;
    }
    if (strlen(result) >= REPLY_RESULT_SIZE) {
        return false;
    }
    if (!ring_add(&ring, &place)) {
        return false;
    }

    strcpy(replies[place].id, id);
    strcpy(replies[place].result, result);
    return true;
}

bool reply_store_oldest(stored_reply_t *stored)
{
    if (ring_is_empty(&ring)) {
        return false;
    }

    *stored = replies[ring_oldest_place(&ring)];
    return true;
}

void reply_store_remove_oldest(void)
{
    ring_remove_oldest(&ring);
}

bool reply_store_remove(const char *id)
{
    if (ring_is_empty(&ring)) {
        return false;
    }
    if (strcmp(replies[ring_oldest_place(&ring)].id, id) != 0) {
        return false;  // not the oldest reply: nothing is removed
    }

    ring_remove_oldest(&ring);
    return true;
}

bool reply_store_is_full(void)
{
    return ring_is_full(&ring);
}

bool reply_store_is_empty(void)
{
    return ring_is_empty(&ring);
}
