#include <stdlib.h>
#include <string.h>

#include "../include/stack.h"


static void StackExpand(Stack *stack);

static void StackShrink(Stack *stack);


Stack StackCtor(size_t capacity)
{
    Stack stack = {};

    stack.capacity = capacity;
    stack.size     = 0;
    stack.data     = (stack_t *)calloc(capacity, sizeof(stack_t));

    return stack;
}


void StackDtor(Stack *stack)
{
    stack->size     = 0;
    stack->capacity = 0;

    free(stack->data);
    stack->data = NULL;
}


static void StackExpand(Stack *stack)
{
    if(stack->size == stack->capacity)
    {
        stack->data = (stack_t *)realloc(stack->data, sizeof(stack_t) * (stack->capacity *= 2));
        memset(stack->data + stack->size, 0, sizeof(stack_t) * stack->size);
    }
}

static void StackShrink(Stack *stack)
{
    if(stack->size * 4 == stack->capacity)
    {
        stack->data = (stack_t *)realloc(stack->data, sizeof(stack_t) * (stack->capacity /= 2));
    }
}


void PushStack(Stack *stack, stack_t val)
{
    stack->data[stack->size] = val;
    stack->size++;

    StackExpand(stack);
}

stack_t PopStack(Stack *stack)
{
    stack->size--;

    stack_t ret_val = stack->data[stack->size];
    stack->data[stack->size] = 0;

    StackShrink(stack);

    return ret_val;
}

stack_t GetStackTop(Stack *stack)
{
    if(stack->size == 0)
        return NULL;
    else
        return stack->data[stack->size - 1];
}