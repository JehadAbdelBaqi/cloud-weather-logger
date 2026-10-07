#ifndef COMMANDS_H
#define COMMANDS_H

// The commands the server can send: what each one does, and the reply it gets.

/**
 * @brief Carries out a command and sends its reply.
 *
 * A command that is not known gets a reply saying so.
 *
 * @param id      The ID of the command, which goes back in the reply
 * @param command The command as it arrived, e.g. "read" or "rate 10"
 */
void commands_run(const char *id, const char *command);

#endif
