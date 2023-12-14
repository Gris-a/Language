#include <stdbool.h>

#include "../include/types.h"

bool IsNumber(Node *node)
{
    return (node->type == NodeType::NUM);
}

bool IsTemporary(Node *node)
{
    return (node->type == NodeType::TMP);
}

bool IsTable(Node *node)
{
    return (node->type == NodeType::TABLE);
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






char GetTemporary(Node *node)
{
    return node->data.tmp;
}

NamesTable *GetTable(Node *node)
{
    return node->data.table;
}

double GetNumber(Node *node)
{
    return node->data.num;
}

Keyword GetKeyword(Node *node)
{
    return node->data.name->value.kword;
}

Operator GetOperator(Node *node)
{
    return node->data.name->value.op;
}

size_t GetFunctionNArgs(Node *node)
{
    return node->data.name->value.func_n_args;
}

double GetVariableValue(Node *node)
{
    return node->data.name->value.var_val;
}

char *GetIdentificator(Node *node)
{
    return node->data.name->ident;
}