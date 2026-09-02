#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

void token_list_init(token_list_t *list)
{
    list->count = 0;
}

void token_add(token_list_t *list,
               token_type_t type,
               const char *text)
{
    if (list->count >= MAX_TOKENS)
        return;

    list->tokens[list->count].type = type;

    strncpy(list->tokens[list->count].text,
            text,
            MAX_TOKEN_LENGTH - 1);

    list->tokens[list->count].text[MAX_TOKEN_LENGTH - 1] = '\0';

    list->count++;
}

void token_print(const token_list_t *list)
{
    for (int i = 0; i < list->count; i++)
    {
        printf("Token %d: type=%d text=\"%s\"\n",
               i,
               list->tokens[i].type,
               list->tokens[i].text);
    }
}

void lexer(const char *input, token_list_t *tokens)
{
    token_list_init(tokens);

    int i = 0;

    while (input[i] != '\0')
    {
        /* Skip whitespace */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /* Pipe */
        if (input[i] == '|')
        {
            token_add(tokens, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        /* Input redirection */
        if (input[i] == '<')
        {
            token_add(tokens, TOKEN_INPUT, "<");
            i++;
            continue;
        }

        /* Output / append */
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                token_add(tokens, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                token_add(tokens, TOKEN_OUTPUT, ">");
                i++;
            }

            continue;
        }

        /* Background */
        if (input[i] == '&')
        {
            token_add(tokens, TOKEN_BACKGROUND, "&");
            i++;
            continue;
        }

        /* Normal word */
        char word[MAX_TOKEN_LENGTH];
        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '|' &&
               input[i] != '<' &&
               input[i] != '>' &&
               input[i] != '&')
        {
            if (j < MAX_TOKEN_LENGTH - 1)
            {
                word[j++] = input[i];
            }

            i++;
        }

        word[j] = '\0';

        if (j > 0)
        {
            token_add(tokens, TOKEN_WORD, word);
        }
    }

    token_add(tokens, TOKEN_END, "");
}
