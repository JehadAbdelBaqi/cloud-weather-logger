#ifndef COMMANDS_H
#define COMMANDS_H

#include "app_state.h"

// The commands. Described in commands.c

void command_help(app_state_t *state, const char *value, char *result);
void command_read(app_state_t *state, const char *value, char *result);
void command_rate(app_state_t *state, const char *value, char *result);
void run_command(app_state_t *state, const char *command, const char *value, char *result);

#endif
