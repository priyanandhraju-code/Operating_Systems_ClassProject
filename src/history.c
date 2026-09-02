#include <stdio.h>
#include <readline/history.h>

void print_history(void)
{
    HIST_ENTRY **hist = history_list();

    if (hist == NULL)
        return;

    for (int i = 0; hist[i] != NULL; i++)
    {
        printf("%d  %s\n", i + 1, hist[i]->line);
    }
}
