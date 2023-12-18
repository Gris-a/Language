#ifndef SPU_H
#define SPU_H

#include <stdbool.h>

#include "../general/general.h"

struct Processor
{
    Stack SPU_stack;
    Stack CALLS_stack;

    data_t registers[REG_COUNT];
    char* STEIN;

    char *name;
    size_t size;
};

const size_t RAM_SIZE = 4000;

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 40;

#define SPU_DUMP(SPU_ptr) LOG("%s:%s:%d:\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                          SPUDump(SPU_ptr)

#ifdef PROTECT
#define SPU_VERIFICATION(SPU_ptr, ret_val_on_fail)   if(!IsSPUValid(SPU_ptr))\
                                                     {\
                                                         LOG("%s:%s: Error: invalid SPU.\n", __FILE__, __PRETTY_FUNCTION__);\
                                                         SPU_DUMP(SPU_ptr);\
                                                         return ret_val_on_fail;\
                                                     }
#else
#define SPU_VERIFICATION(...)
#endif

Processor SPUCtor(void);

int SPUDtor(Processor *SPU);

int Execute(const char *const path, Processor *SPU);

void SPUDump(Processor *SPU);

#ifdef PROTECT
bool IsSPUValid(Processor *SPU);
#endif

#endif //SPU_H