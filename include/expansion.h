#ifndef EXPANSION_H
#define EXPANSION_H

#define MAX_EXPANDED_LENGTH 1024

int expand_variables(const char *input,
                     char *output,
                     int output_size);

#endif
