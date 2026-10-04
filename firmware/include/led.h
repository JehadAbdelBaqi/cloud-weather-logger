#ifndef LED_H
#define LED_H

/**
 * @brief Sets up the pin of the on-board user LED as an output, LED off.
 *
 * Call once before led_toggle().
 */
void led_init(void);

/**
 * @brief Switches the LED on if it is off, and off if it is on.
 */
void led_toggle(void);

#endif
