#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

/*
 * Execute a single command.
 *
 * Handles:
 *      - built-ins
 *      - external commands
 *      - input/output redirection
 */
int execute_command(command_t *cmd);


/*
 * Execute a complete pipeline.
 *
 * Example:
 *
 *      ls -l | grep .c | wc -l
 */
int execute_pipeline(pipeline_t *pipeline);

#endif
