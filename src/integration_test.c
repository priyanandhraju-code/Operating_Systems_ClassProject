#include <stdio.h>

#include "expansion.h"
#include "lexer.h"
#include "parser.h"

int main(void)
{
    const char *input = "echo $HOME > output.txt";

    char expanded[MAX_EXPANDED_LENGTH];

    token_list_t tokens;
    pipeline_t pipeline;

    printf("========== ORIGINAL INPUT ==========\n");
    printf("%s\n\n", input);

    /* Step 1: Environment expansion */
    if (!expand_variables(input,
                          expanded,
                          sizeof(expanded)))
    {
        printf("Expansion failed.\n");
        return 1;
    }

    printf("========== EXPANDED INPUT ==========\n");
    printf("%s\n\n", expanded);

    /* Step 2: Lexing */
    lexer(expanded, &tokens);

    printf("========== TOKENS ==========\n");
    token_print(&tokens);

    /* Step 3: Parsing */
    printf("\n========== PARSED PIPELINE ==========\n");

    if (parser(&tokens, &pipeline))
    {
        pipeline_print(&pipeline);
    }
    else
    {
        printf("Parser failed.\n");
        return 1;
    }

    return 0;
}
