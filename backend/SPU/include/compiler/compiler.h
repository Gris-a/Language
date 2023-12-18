#ifndef COMPILER_H
#define COMPILER_H

#include "../general/general.h"

const int N_COMPILATIONS = 2;
const int MAX_LABELS    = 50;
const int MAX_LEN      = 100;

struct Label
{
    char name[MAX_LEN];
    size_t pos;
};

int Assembler(const char * const path);

#endif //COMPILER_H