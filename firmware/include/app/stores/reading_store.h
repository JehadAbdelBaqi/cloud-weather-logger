#ifndef READING_STORE_H
#define READING_STORE_H

#include <stdbool.h>
#include <stdint.h>

#include "reading.h"

/** A reading as it is kept in the store: the measurement, with its number and the time it was taken. */
typedef struct {
    uint32_t seq;       // sequence number, from 1 at power-on
    uint32_t time;      // Unix time in seconds
    reading_t reading;  // the measurement
} stored_reading_t;

/**
 * @brief Puts a reading into the store, behind the ones already there, and gives it the next
 *        sequence number.
 *
 * The sequence number starts at 1 at power-on. A reading that is not stored uses none.
 *
 * @param reading The reading; a copy is kept
 * @param time    The time the reading was taken, as Unix time in seconds
 * @return true if it was stored, false if the store is full
 */
bool reading_store_put(const reading_t *reading, uint32_t time);

/**
 * @brief Gives the oldest reading in the store, and leaves it there.
 *
 * @param stored Filled in with a copy of the oldest reading, its sequence number and its time;
 *               left as it was if the store is empty
 * @return true if there was a reading, false if the store is empty
 */
bool reading_store_oldest(stored_reading_t *stored);

/**
 * @brief Removes the oldest reading from the store. Does nothing if the store is empty.
 */
void reading_store_remove_oldest(void);

/**
 * @brief Removes the reading with a given sequence number from the store, if it is the oldest one.
 *
 * Readings leave the store oldest first, so a number that is not the oldest's removes nothing.
 *
 * @param seq The sequence number of the reading to remove
 * @return true if the reading was removed
 */
bool reading_store_remove(uint32_t seq);

/**
 * @brief Says whether the store has no room for another reading.
 *
 * @return true if the store is full
 */
bool reading_store_is_full(void);

/**
 * @brief Says whether the store holds no readings.
 *
 * @return true if the store is empty
 */
bool reading_store_is_empty(void);

#endif
