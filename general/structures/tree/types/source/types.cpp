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
    return ((node->type            == NodeType::NAME) &&
            (node->data.name->type == NameType::KWORD));
}

bool IsOperator(Node *node)
{
    return ((node->type            == NodeType::NAME) &&
            (node->data.name->type == NameType::OP));
}

bool IsFunction(Node *node)
{
    return ((node->type            == NodeType::NAME) &&
            (node->data.name->type == NameType::FUNC));
}

bool IsVariable(Node *node)
{
    return ((node->type            == NodeType::NAME) &&
            (node->data.name->type == NameType::VAR));
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
    return node->data.name->val.kword;
}

Operator GetOperator(Node *node)
{
    return node->data.name->val.op;
}

size_t GetNArgs(Node *node)
{
    return node->data.name->val.n_args;
}

double GetVariableValue(Node *node)
{
    return node->data.name->val.var_val;
}

char *GetIdentificator(Node *node)
{
    return node->data.name->ident;
}