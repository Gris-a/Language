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
    NAME  = 2,
    WORD  = 3
};

union node_t
{
    char        *word;
    Name        *name;
    double        num;
    SpecialChar sp_ch;
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
    Keyword   kword;
    Operator     op;
    size_t   n_args;
    double  var_val;
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

double GetVariableValue(Node *node);

size_t GetNArgs(Node *node);

char *GetIdentificator(Node *node);

#endif //TYPES_H