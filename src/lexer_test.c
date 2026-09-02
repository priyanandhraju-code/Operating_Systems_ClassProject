#include <stdio.h>

#include "lexer.h"

int main(void)
{
    token_list_t tokens;

    lexer("ls -l | grep .c >> output.txt &", &tokens);

    token_print(&tokens);

    return 0;
}
