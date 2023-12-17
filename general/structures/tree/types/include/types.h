#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>


struct Node;
struct Tree;

struct Name;
struct NamesTable;

struct Token;


///LANGUAGE///

#define KEYWORD(enum, ...) enum,
enum class Keyword
{
    #include "../../../../language/keywords.h"
};
#undef KEYWORD

#define OPERATOR(enum, ...) enum,
enum class Operator
{
    #include "../../../../language/operators.h"
};
#undef OPERATOR

#define SPECIAL_CH(enum, ...) enum,
enum class SpecialChar
{
    #include "../../../../language/special_ch.h"
};
#undef SPECIAL_CH

///TREE///

enum class NodeType
{
    SP_CH = 0,
    NUM   = 1,
    VAR   = 2,
    KWORD = 3,
    OP    = 4,
    FUNC  = 5,
    WORD  = 6
};

union node_t
{
    SpecialChar sp_ch;
    double        num;
    size_t     var_id;
    Keyword     kword;
    Operator       op;
    char        *func;
    char        *word;
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
    Node *root;
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
    Keyword kword;
    Operator   op;
    size_t n_args;
    size_t var_id;
};

struct Name
{
    char   *ident;
    NameType type;
    name_t    val;
};

struct NamesTable
{
    Name *names;

    size_t capacity;
    size_t     size;
};

///TYPE CHECK FUNCTIONS///

bool IsNumber(Node *node);

bool IsSpecialChar(Node *node);

bool IsKeyword(Node *node);

bool IsWord(Node *node);

bool IsOperator(Node *node);

bool IsVariable(Node *node);

bool IsFunction(Node *node);

///NODE DATA GETTERS

SpecialChar GetSpecialChar(Node *node);

double GetNumber(Node *node);

Keyword GetKeyword(Node *node);

Operator GetOperator(Node *node);

char *GetWord(Node *node);

size_t GetVariable(Node *node);

char *GetFunction(Node *node);

#endif //TYPES_H