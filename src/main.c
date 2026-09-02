#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "history.h"
#include "expansion.h"
#include "lexer.h"
#include "parser.h"
#include "executor.h"


int main(void)
{
    printf("=====================================\n");
    printf("          Shellforge\n");
    printf("    A Unix Style Shell written in C\n");
    printf("=====================================\n");

    using_history();

    while (1)
    {
        char *line = readline("shellforge$ ");

        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }

        /*
         * Ignore empty input.
         */
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        /*
         * Add command to history.
         */
        add_history(line);

        /*
         * ------------------------------------------------
         * ENVIRONMENT VARIABLE EXPANSION
         * ------------------------------------------------
         */
        char expanded[MAX_EXPANDED_LENGTH];

        if (!expand_variables(line,
                              expanded,
                              MAX_EXPANDED_LENGTH))
        {
            fprintf(stderr, "Expansion failed.\n");
            free(line);
            continue;
        }

        /*
         * ------------------------------------------------
         * LEXER
         * ------------------------------------------------
         */
        token_list_t tokens;

        lexer(expanded, &tokens);

        /*
         * ------------------------------------------------
         * PARSER
         * ------------------------------------------------
         */
        pipeline_t pipeline;

        if (!parser(&tokens, &pipeline))
        {
            fprintf(stderr, "Parser failed.\n");
            free(line);
            continue;
        }

        /*
         * ------------------------------------------------
         * EXECUTOR
         * ------------------------------------------------
         *
         * For now, execute_command() handles:
         *
         *      - built-in commands
         *      - external commands
         *
         * Pipeline execution will be added later.
         */
        if (pipeline.command_count == 1)
        {
            int result = execute_command(&pipeline.commands[0]);

            /*
             * result == 1 means the exit built-in
             * requested that the shell terminate.
             */
            if (result == 1 &&
                pipeline.commands[0].argc > 0 &&
                strcmp(pipeline.commands[0].argv[0], "exit") == 0)
            {
                free(line);
                printf("Exiting...\n");
                break;
            }
        }
        else
        {
            /*
             * Pipeline execution will be implemented
             * in the next part of Milestone 4.1.
             */
            printf("Pipeline execution not implemented yet.\n");
        }

        free(line);
    }

    return 0;
}
