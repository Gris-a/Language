#include <stdbool.h>

#include "../include/types.h"

bool IsNumber(Node *node)
{
    return (node->type == NodeType::NUM);
}

bool IsSpecialChar(Node *node)
{
    return (node->type == NodeType::SP_CH);
}

bool IsWord(Node *node)
{
    return (node->type == NodeType::WORD);
}

bool IsKeyword(Node *node)
{
    return (node->type == NodeType::KWORD);
}

bool IsOperator(Node *node)
{
    return (node->type == NodeType::OP);
}

bool IsFunction(Node *node)
{
    return (node->type == NodeType::FUNC);
}

bool IsVariable(Node *node)
{
    return (node->type == NodeType::VAR);
}







double GetNumber(Node *node)
{
    return node->data.num;
}

SpecialChar GetSpecialChar(Node *node)
{
    return node->data.sp_ch;
}

char *GetWord(Node *node)
{
    return node->data.word;
}

Keyword GetKeyword(Node *node)
{
    return node->data.kword;
}

Operator GetOperator(Node *node)
{
    return node->data.op;
}

char *GetFunction(Node *node)
{
    return node->data.func;
}

size_t GetVariable(Node *node)
{
    return node->data.var_id;
}