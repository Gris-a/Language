#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "../../general/tree/include/tree.h"

struct Function
{
    char *func_name;
    size_t   n_vars;
    size_t var_size;
    int  *variables;
};

void Translator(const char *file_name);

#endif //TRANSLATOR_H