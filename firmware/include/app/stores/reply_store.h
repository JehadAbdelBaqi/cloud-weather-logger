#ifndef REPLY_STORE_H
#define REPLY_STORE_H

#include <stdbool.h>

#include "link_framing.h"

// Room for the ID of a command: up to 16 characters, then the ending '\0'
#define REPLY_ID_SIZE 17

// Room for the result of a reply: what is left of a line after "R,", the longest ID and the
// separator before the result, then the ending '\0'
#define REPLY_RESULT_SIZE (LINK_LINE_MAX_LEN - 3 - (REPLY_ID_SIZE - 1) + 1)

/** A reply as it is kept in the store: the ID of the command it answers, and its result. */
typedef struct {
    char id[REPLY_ID_SIZE];          // the ID of the command, as text
    char result[REPLY_RESULT_SIZE];  // the result, e.g. "ok"
} stored_reply_t;

/**
 * @brief Puts a reply into the store, behind the ones already there.
 *
 * @param id     The ID of the command the reply answers; a copy is kept
 * @param result The result; a copy is kept
 * @return true if it was stored; false if the store is full, or the ID or the result does not fit
 */
bool reply_store_put(const char *id, const char *result);

/**
 * @brief Gives the oldest reply in the store, and leaves it there.
 *
 * @param stored Filled in with a copy of the oldest reply; left as it was if the store is empty
 * @return true if there was a reply, false if the store is empty
 */
bool reply_store_oldest(stored_reply_t *stored);

/**
 * @brief Removes the oldest reply from the store. Does nothing if the store is empty.
 */
void reply_store_remove_oldest(void);

/**
 * @brief Removes the reply with a given ID from the store, if it is the oldest one.
 *
 * Replies leave the store oldest first, so an ID that is not the oldest's removes nothing.
 *
 * @param id The ID of the reply to remove
 * @return true if the reply was removed
 */
bool reply_store_remove(const char *id);

/**
 * @brief Says whether the store has no room for another reply.
 *
 * @return true if the store is full
 */
bool reply_store_is_full(void);

/**
 * @brief Says whether the store holds no replies.
 *
 * @return true if the store is empty
 */
bool reply_store_is_empty(void);

#endif
