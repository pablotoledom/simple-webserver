#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexiserver.h"

void parse_config_file(int *LPORT, char *WEB_ROOT, int *LBUFSIZE, char *LEXISERVER) {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("ERROR OPENING CONFIG. FILE");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        char key[64], value[256];
        if (sscanf(line, "%63[^=]=%255[^\n]", key, value) == 2) {
            if (strcmp(key, "LPORT") == 0)
                *LPORT = atoi(value);
            else if (strcmp(key, "WEB_ROOT") == 0)
                strcpy(WEB_ROOT, value);
            else if (strcmp(key, "LBUFSIZE") == 0)
                *LBUFSIZE = atoi(value);
            else if (strcmp(key, "LEXISERVER") == 0)
                strcpy(LEXISERVER, value);
        }
    }

    fclose(file);
}
