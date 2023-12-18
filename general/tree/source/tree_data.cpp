#include <limits.h>
#include <math.h>

#include "../include/tree.h"

bool IsNumber(Node *node)
{
    ASSERT(node, return false);
    return (node->type == NodeType::NUM);
}

bool IsSpecialChar(Node *node)
{
    ASSERT(node, return false);
    return (node->type == NodeType::SP_CH);
}

bool IsWord(Node *node)
{
    ASSERT(node, return false);
    return (node->type == NodeType::WORD);
}

bool IsKeyword(Node *node)
{
    ASSERT(node, return false);
    return (node->type == NodeType::KWORD);
}

bool IsOperator(Node *node)
{
    ASSERT(node, return false);
    return (node->type == NodeType::OP);
}

bool IsFunction(Node *node)
{
    ASSERT(node, return false);
    return (node->type == NodeType::FUNC);
}

bool IsVariable(Node *node)
{
    ASSERT(node, return false);
    return (node->type == NodeType::VAR);
}







double GetNumber(Node *node)
{
    ASSERT(node && IsNumber(node), return NAN);
    return node->data.num;
}

SpecialChar GetSpecialChar(Node *node)
{
    ASSERT(node && IsSpecialChar(node), return SpecialChar::NOT_SP_CH);
    return node->data.sp_ch;
}

char *GetWord(Node *node)
{
    ASSERT(node && IsWord(node), return NULL);
    return node->data.word;
}

Keyword GetKeyword(Node *node)
{
    ASSERT(node && IsKeyword(node), return Keyword::NOT_KWORD);
    return node->data.kword;
}

Operator GetOperator(Node *node)
{
    ASSERT(node && IsOperator(node), return Operator::NOT_OP);
    return node->data.op;
}

char *GetFunction(Node *node)
{
    ASSERT(node && IsFunction(node), return NULL);
    return node->data.func;
}

size_t GetVariable(Node *node)
{
    ASSERT(node && IsVariable(node), return ULLONG_MAX);
    return node->data.var_id;
}
