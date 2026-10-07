#ifndef STATUS_H
#define STATUS_H

/**
 * @brief Shows the link to the bridge on the user LED: a fast blink while the connection is being
 *        made, a slow blink once it is.
 *
 * Call on every pass of the main loop. Does not wait.
 */
void status_poll(void);

#endif
