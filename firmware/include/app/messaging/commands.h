#ifndef COMMANDS_H
#define COMMANDS_H

// Handling commands: a "do this" from the server, what each one does, and the reply it gets.

/**
 * @brief Handles a command received from the server: C,<id>,<command>.
 *
 * The command is carried out and answered with a reply line that carries the same ID. A command
 * that is not known gets a reply saying so. A command with no ID cannot be answered, so it is
 * ignored.
 *
 * @param line The line received, starting with PROTOCOL_LETTER_COMMAND
 */
void handle_command(const char *line);

#endif
