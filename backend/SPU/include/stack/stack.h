#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

#include "../../../../general/log/log.h"

typedef double data_t;
#define DATA_FORMAT "%lg"

const size_t BASE_CAPACITY = 2;

struct Stack
{
    size_t size;
    size_t capacity;

    data_t *data;
};

#define STACK_DUMP(stack_ptr) LOG("Called from %s:%s:%d:\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                              StackDump(stack_ptr)

#ifdef PROTECT
#define STACK_VERIFICATION(stack_ptr, ret_val_on_fail) if(!IsStackValid(stack_ptr))\
                                                      {\
                                                          LOG("Error: invalid stack.\n");\
                                                          STACK_DUMP(stack_ptr);\
                                                          return ret_val_on_fail;\
                                                      }
#else
#define STACK_VERIFICATION(...)
#endif

Stack StackCtor(const size_t capacity = BASE_CAPACITY);

int StackDtor(Stack *stack);

int PushStack(Stack *stack, const data_t val);

int PopStack(Stack *stack, data_t *ret_val = NULL);

int ClearStack(Stack *stack);

void StackDump(Stack *stack);

#ifdef PROTECT
bool IsStackValid(Stack *stack);
#endif

#endif //STACK_H
