#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "expansion.h"

int expand_variables(const char *input,
                     char *output,
                     int output_size)
{
    int i = 0;
    int j = 0;

    while (input[i] != '\0')
    {
        /* Normal character */
        if (input[i] != '$')
        {
            if (j >= output_size - 1)
            {
                return 0;
            }

            output[j++] = input[i++];
            continue;
        }

        /* '$' found */
        i++;

        /* '$' at the end */
        if (input[i] == '\0')
        {
            if (j >= output_size - 1)
            {
                return 0;
            }

            output[j++] = '$';
            break;
        }

        /*
         * A variable name must start with
         * a letter or underscore.
         */
        if (!isalpha((unsigned char)input[i]) &&
            input[i] != '_')
        {
            if (j >= output_size - 1)
            {
                return 0;
            }

            output[j++] = '$';
            continue;
        }

        /* Read variable name */
        char variable[128];
        int k = 0;

        while (input[i] != '\0' &&
               (isalnum((unsigned char)input[i]) ||
                input[i] == '_'))
        {
            if (k < (int)sizeof(variable) - 1)
            {
                variable[k++] = input[i];
            }

            i++;
        }

        variable[k] = '\0';

        /* Get environment variable */
        const char *value = getenv(variable);

        if (value == NULL)
        {
            continue;
        }

        int value_length = strlen(value);

        if (j + value_length >= output_size)
        {
            return 0;
        }

        strcpy(&output[j], value);

        j += value_length;
    }

    output[j] = '\0';

    return 1;
}
