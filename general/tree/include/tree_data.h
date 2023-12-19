#ifndef TREE_DATA_H
#define TREE_DATA_H

#include <stdbool.h>
#include <stddef.h>

#include "tree_struct.h"

bool IsNumber(Node *node);

bool IsSpecialChar(Node *node);

bool IsKeyword(Node *node);

bool IsWord(Node *node);

bool IsOperator(Node *node);

bool IsVariable(Node *node);

bool IsFunction(Node *node);


SpecialChar GetSpecialChar(Node *node);

double GetNumber(Node *node);

Keyword GetKeyword(Node *node);

Operator GetOperator(Node *node);

char *GetWord(Node *node);

size_t GetVariable(Node *node);

char *GetFunction(Node *node);

#endif //TREE_DATA_H