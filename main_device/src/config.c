#include "config.h"
#include <stdio.h>
#include <stdlib.h>

int read_config(const char* filename, int* pins, int* polling_rates) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        return 1;
    }

    if (fscanf(file, "%d %d %d %d %d %d", 
               &pins[0], &pins[1], &polling_rates[0], 
               &pins[2], &pins[3], &polling_rates[1]) != 6) {
        fprintf(stderr, "Invalid config file format\n");
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}
