#ifndef STACK_H
#define STACK_H

#include <stddef.h>

#include "../../general/log/log.h"

typedef void *stack_t;

const size_t BASE_CAPACITY = 2;

struct Stack
{
    size_t size;
    size_t capacity;

    stack_t *data;
};

Stack StackCtor(size_t capacity = BASE_CAPACITY);

void StackDtor(Stack *stack);

void PushStack(Stack *stack, stack_t val);

stack_t PopStack(Stack *stack);

stack_t GetStackTop(Stack *stack);

#endif //STACK_H
