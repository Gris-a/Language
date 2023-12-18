#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../../include/stack/stack.h"

Stack StackCtor(const size_t capacity)
{
    if(capacity == 0)
    {
        LOG("Capacity should be greater than zero.\n");
        return {};
    }

    Stack stack = {};

    stack.capacity = capacity;

    stack.data     = (data_t *)calloc(capacity * sizeof(data_t), sizeof(char));
    ASSERT(stack.data, return {});

    return stack;
}


int StackDtor(Stack *stack)
{
    STACK_VERIFICATION(stack, EXIT_FAILURE);

    stack->size     = 0;
    stack->capacity = 0;

    free(stack->data);
    stack->data = NULL;

    return EXIT_SUCCESS;
}


static int StackExpand(Stack *stack)
{
    if(stack->size == stack->capacity)
    {
        data_t *data_r = (data_t *)realloc(stack->data, sizeof(data_t) * stack->capacity * 2);
        ASSERT(data_r, return EXIT_FAILURE);

        stack->capacity *= 2;

        stack->data = data_r;

        memset(stack->data + stack->size, 0, sizeof(data_t) * stack->size);
    }

    return EXIT_SUCCESS;
}

static int StackShrink(Stack *stack)
{
    if(stack->size * 4 == stack->capacity)
    {
        data_t *data_r = (data_t *)realloc(stack->data, sizeof(data_t) * stack->capacity / 2);
        ASSERT(data_r, return EXIT_FAILURE);

        stack->capacity /= 2;

        stack->data = data_r;
    }

    return EXIT_SUCCESS;
}


int PushStack(Stack *stack, const data_t val)
{
    STACK_VERIFICATION(stack, EXIT_FAILURE);

    stack->data[stack->size] = val;
    stack->size++;

    int exit_status = StackExpand(stack);

    return exit_status;
}

int PopStack(Stack *stack, data_t *ret_val)
{
    STACK_VERIFICATION(stack, EXIT_FAILURE);

    if(stack->size == 0)
    {
        LOG("Stack underflow.\n");
        return EXIT_FAILURE;
    }

    stack->size--;

    if(ret_val) *ret_val = stack->data[stack->size];
    stack->data[stack->size] = 0;

    int exit_status = StackShrink(stack);

    return exit_status;
}


int ClearStack(Stack *stack)
{
    STACK_VERIFICATION(stack, EXIT_FAILURE);

    memset(stack->data, 0, sizeof(data_t) * stack->size);

    stack->size = 0;

    int exit_status = StackShrink(stack);

    return exit_status;
}


void StackDump(Stack *stack)
{
    LOG("Stack[%p]:\n", stack);

    if(!stack) return;

    LOG("\tsize     = %zu;\n"
        "\tcapacity = %zu;\n"
        "\tdata[%p]:      \n", stack->size, stack->capacity, stack->data);

    if(!stack->data) return;

    for(size_t i = 0; i < stack->capacity; i++)
    {
        LOG("\t\t");
        LOG((i < stack->size) ? "*" : " ");
        LOG("[%3zu] = " DATA_FORMAT "\n", i, stack->data[i]);
    }
}


#ifdef PROTECT
bool IsStackValid(Stack *stack)
{
    ASSERT(stack      , return false);
    ASSERT(stack->data, return false);

    ASSERT(stack->capacity <= INT_MAX    , return false);
    ASSERT(stack->capacity != 0          , return false);
    ASSERT(stack->size <= stack->capacity, return false);

    return true;
}
#endif