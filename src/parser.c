#include <stdio.h>
#include <string.h>

#include "parser.h"

int parser(token_list_t *tokens,
           pipeline_t *pipeline)
{
    if (tokens == NULL || pipeline == NULL)
    {
        return 0;
    }

    pipeline->command_count = 1;

    command_t *cmd = &pipeline->commands[0];

    cmd->argc = 0;
    cmd->input[0] = '\0';
    cmd->output[0] = '\0';
    cmd->append = 0;
    cmd->background = 0;

    for (int i = 0; i < tokens->count; i++)
    {
        token_t *token = &tokens->tokens[i];

        /* Normal word */
        if (token->type == TOKEN_WORD)
        {
            if (cmd->argc >= MAX_ARGS - 1)
            {
                return 0;
            }

            cmd->argv[cmd->argc] = token->text;
            cmd->argc++;

            cmd->argv[cmd->argc] = NULL;
        }

        /* Pipe */
        else if (token->type == TOKEN_PIPE)
        {
            if (cmd->argc == 0)
            {
                return 0;
            }

            if (pipeline->command_count >= MAX_COMMANDS)
            {
                return 0;
            }

            cmd = &pipeline->commands[pipeline->command_count];

            cmd->argc = 0;
            cmd->input[0] = '\0';
            cmd->output[0] = '\0';
            cmd->append = 0;
            cmd->background = 0;

            pipeline->command_count++;
        }

        /* Input redirection */
        else if (token->type == TOKEN_INPUT)
        {
            if (i + 1 >= tokens->count)
            {
                return 0;
            }

            if (tokens->tokens[i + 1].type != TOKEN_WORD)
            {
                return 0;
            }

            strcpy(cmd->input,
                   tokens->tokens[i + 1].text);

            i++;
        }

        /* Output redirection */
        else if (token->type == TOKEN_OUTPUT)
        {
            if (i + 1 >= tokens->count)
            {
                return 0;
            }

            if (tokens->tokens[i + 1].type != TOKEN_WORD)
            {
                return 0;
            }

            strcpy(cmd->output,
                   tokens->tokens[i + 1].text);

            cmd->append = 0;

            i++;
        }

        /* Append redirection */
        else if (token->type == TOKEN_APPEND)
        {
            if (i + 1 >= tokens->count)
            {
                return 0;
            }

            if (tokens->tokens[i + 1].type != TOKEN_WORD)
            {
                return 0;
            }

            strcpy(cmd->output,
                   tokens->tokens[i + 1].text);

            cmd->append = 1;

            i++;
        }

        /* End */
        else if (token->type == TOKEN_END)
        {
            break;
        }
    }

    return 1;
}

void pipeline_print(const pipeline_t *pipeline)
{
    printf("Number of commands: %d\n\n",
           pipeline->command_count);

    for (int i = 0; i < pipeline->command_count; i++)
    {
        const command_t *cmd = &pipeline->commands[i];

        printf("Command %d:\n", i + 1);

        for (int j = 0; j < cmd->argc; j++)
        {
            printf("  argv[%d] = \"%s\"\n",
                   j,
                   cmd->argv[j]);
        }

        if (cmd->input[0] != '\0')
        {
            printf("  input = \"%s\"\n",
                   cmd->input);
        }

        if (cmd->output[0] != '\0')
        {
            printf("  output = \"%s\"\n",
                   cmd->output);

            printf("  append = %d\n",
                   cmd->append);
        }

        printf("\n");
    }
}
