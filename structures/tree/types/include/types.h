#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>


struct Node;
struct Tree;

struct Function;
struct Name;
struct NamesTable;

struct Token;


///LANGUAGE///

#define KEYWORD(enum, ...) enum,
enum class Keyword
{
    #include "../../../../general/language/keywords.h"
};
#undef KEYWORD

#define OPERATOR(enum, ...) enum,
enum class Operator
{
    #include "../../../../general/language/operators.h"
};
#undef OPERATOR

///TREE///

enum class NodeType
{
    TMP   = 0,
    NUM   = 1,
    NAME  = 2,
    TABLE = 3
};

union node_t
{
    char          tmp;
    double        num;
    Name        *name;
    NamesTable *table;
};

struct Node
{
    NodeType type;
    node_t   data;

    Node  *left;
    Node *right;
};

struct Tree
{
    Node         *root;
    NamesTable   *lang;
    NamesTable *global;
};

///VARIABLES TABLE///

enum class NameType
{
    KWORD = 0,
    OP    = 1,
    FUNC  = 2,
    VAR   = 3,
};

union name_t
{
    Keyword      kword;
    Operator        op;
    size_t func_n_args;
    double     var_val;
};

struct Name
{
    char   *ident;
    NameType type;
    name_t  value;
};

struct NamesTable
{
    Name *names;

    size_t capacity;
    size_t     size;
};

///TYPE CHECK FUNCTIONS///

bool IsNumber(Node *node);

bool IsTemporary(Node *node);

bool IsTable(Node *node);

bool IsKeyword(Node *node);

bool IsOperator(Node *node);

bool IsVariable(Node *node);

bool IsFunction(Node *node);

///NODE DATA GETTERS

char GetTemporary(Node *node);

double GetNumber(Node *node);

NamesTable *GetTable(Node *node);

Keyword GetKeyword(Node *node);

Operator GetOperator(Node *node);

double GetVariableValue(Node *node);

size_t GetFunctionNArgs(Node *node);

char *GetIdentificator(Node *node);

#endif //TYPES_H