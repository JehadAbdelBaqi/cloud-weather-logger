#ifndef LINK_BUFFER_H
#define LINK_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

// A buffer for received bytes, between the interrupt that takes each byte from the UART as it
// arrives and the main loop that reads them later. Bytes come out in the order they went in.
// One side only puts and the other only takes, so neither has to stop the other.

// How many places the buffer has. It holds one byte fewer than this. At 115200 baud that is room
// for the bytes that arrive in about 44 ms, longer than the main loop's longest pass
#define LINK_BUFFER_SIZE 512

/** The buffer. Start with every field zero. */
typedef struct {
    uint8_t bytes[LINK_BUFFER_SIZE];
    volatile uint32_t put_place;   // where the next byte put in goes
    volatile uint32_t take_place;  // where the next byte taken comes from
} link_buffer_t;

/**
 * @brief Puts one byte in the buffer.
 *
 * @param buffer The buffer
 * @param byte   The byte
 * @return true if the byte was put in, false if the buffer is full and the byte was dropped
 */
bool link_buffer_put(link_buffer_t *buffer, uint8_t byte);

/**
 * @brief Takes the oldest byte out of the buffer.
 *
 * @param buffer The buffer
 * @param byte   Filled in with the byte, when there is one
 * @return true if a byte was taken, false if the buffer is empty
 */
bool link_buffer_take(link_buffer_t *buffer, uint8_t *byte);

#endif
