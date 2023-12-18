#include <stdlib.h>
#include <string.h>

#include "stack.h"


static void StackExpand(Stack *stack);

static void StackShrink(Stack *stack);


Stack StackCtor(size_t capacity)
{
    Stack stack = {};

    stack.capacity = capacity;
    stack.size     = 0;
    stack.data     = (stack_t *)calloc(capacity, sizeof(stack_t));
    if(!stack.data)
    {
        LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);

        return {};
    }

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
        stack_t *temp = (stack_t *)realloc(stack->data, sizeof(stack_t) * (stack->capacity *= 2));
        if(!temp)
        {
            LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);

            return;
        }

        stack->data = temp;
        memset(stack->data + stack->size, 0, sizeof(stack_t) * stack->size);
    }
}

static void StackShrink(Stack *stack)
{
    if(stack->size * 4 == stack->capacity)
    {
        stack_t *temp = (stack_t *)realloc(stack->data, sizeof(stack_t) * (stack->capacity /= 2));
        if(!temp)
        {
            LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);

            return;
        }

        stack->data = temp;
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