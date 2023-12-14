#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdbool.h>

#define PROTECT

#ifndef LOG_CPP
extern FILE *LOG_FILE;
#endif

#define LOG(...) fprintf(LOG_FILE, __VA_ARGS__)
#define LOGS(string) fputs(string, LOG_FILE)
#define LOG_START LOG("\n////////////////////////////////////////////////////////////////////////////////////////////LOG START////////////////////////////////////////////////////////////////////////////////////////////\n")
#define LOG_END   LOG("\n/////////////////////////////////////////////////////////////////////////////////////////////LOG END/////////////////////////////////////////////////////////////////////////////////////////////\n")

#ifdef PROTECT
#define ASSERT(condition, action) if(!(condition))\
                                  {\
                                      LOG ("%s:%s:%d: Assertion cathced at ASSERT(", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                                      LOGS(#condition);\
                                      LOG (", ...);\n");\
                                      \
                                      action;\
                                  }
#else
#define ASSERT(condition, action)
#endif

FILE *OpenLog(void);

void CloseLog(void);

#endif //LOG_H