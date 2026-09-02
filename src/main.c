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
        /*
         * -------------------------------------------
         * READ COMMAND
         * -------------------------------------------
         */
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
         * -------------------------------------------
         * ENVIRONMENT VARIABLE EXPANSION
         * -------------------------------------------
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
         * -------------------------------------------
         * LEXER
         * -------------------------------------------
         */
        token_list_t tokens;


        lexer(expanded, &tokens);


        /*
         * -------------------------------------------
         * PARSER
         * -------------------------------------------
         */
        pipeline_t pipeline;


        if (!parser(&tokens, &pipeline))
        {
            fprintf(stderr, "Parser failed.\n");
            free(line);
            continue;
        }


        /*
         * -------------------------------------------
         * HISTORY COMMAND
         * -------------------------------------------
         */
        if (pipeline.command_count == 1 &&
            pipeline.commands[0].argc > 0 &&
            strcmp(pipeline.commands[0].argv[0], "history") == 0)
        {
            print_history();

            free(line);
            continue;
        }


        /*
         * -------------------------------------------
         * EXIT COMMAND
         * -------------------------------------------
         *
         * execute_pipeline() handles a single
         * built-in command.
         */
        if (pipeline.command_count == 1 &&
            pipeline.commands[0].argc > 0 &&
            strcmp(pipeline.commands[0].argv[0], "exit") == 0)
        {
            free(line);
            printf("Exiting...\n");
            break;
        }


        /*
         * -------------------------------------------
         * EXECUTOR
         * -------------------------------------------
         *
         * This now handles:
         *
         *      single command
         *      external command
         *      redirection
         *      pipeline
         */
        execute_pipeline(&pipeline);


        free(line);
    }


    return 0;
}
