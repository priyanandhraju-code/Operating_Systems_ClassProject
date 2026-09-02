#include <stdio.h>

#include "lexer.h"
#include "parser.h"

int main(void)
{
    token_list_t tokens;
    pipeline_t pipeline;

    const char *input = "ls < input.txt";

    printf("Input: %s\n\n", input);

    lexer(input, &tokens);

    printf("========== TOKENS ==========\n");
    token_print(&tokens);

    printf("\n========== PARSED COMMAND ==========\n");

    if (parser(&tokens, &pipeline))
    {
        pipeline_print(&pipeline);
    }
    else
    {
        printf("Parser failed.\n");
    }

    return 0;
}
