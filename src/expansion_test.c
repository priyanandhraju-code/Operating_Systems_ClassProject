#include <stdio.h>

#include "expansion.h"

int main(void)
{
    char output[MAX_EXPANDED_LENGTH];

    const char *input = "$";

    printf("Input : %s\n", input);

    if (expand_variables(input,
                         output,
                         sizeof(output)))
    {
        printf("Output: %s\n", output);
    }
    else
    {
        printf("Expansion failed.\n");
    }

    return 0;
}
