#define LOG_CPP

#include <stdio.h>
#include <stdlib.h>

#include "log.h"

FILE *LOG_FILE = OpenLog();

FILE *OpenLog(void)
{
    FILE *log_file = fopen("log.log", "wb");

    if(log_file == NULL)
    {
        fprintf(stderr, "Can`t open log-file.\n"
                        "Using stderr insead.\n");
        return stderr;
    }

    atexit(CloseLog);

    setbuf(log_file, NULL);

    return log_file;
}


void CloseLog(void)
{
    fflush(LOG_FILE);

    fclose(LOG_FILE);
}

#undef LOG_CPP