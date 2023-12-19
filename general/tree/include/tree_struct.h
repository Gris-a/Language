#ifndef TREE_STRUCT_H
#define TREE_STRUCT_H

#include <stdbool.h>
#include <stddef.h>

#include "../../log/log.h"
#include "../../language/types.h"

enum class NodeType
{
    SP_CH    = 0,
    NUM      = 1,
    VAR      = 2,
    KWORD    = 3,
    OP       = 4,
    FUNC     = 5,
    WORD     = 6,
    NOT_NODE = 7
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

Tree ReadTree(const char *file_name);

Node *SubTreeCopy(Node *node);


void SubTreeDtor(Node *sub_tree);

int TreeDtor(Tree *tree);


Node *NodeCtor(node_t val, NodeType type, Node *left = NULL, Node *right = NULL);

int NodeDtor(Node *node);


void TreeLog(Tree *tree);

void TreeDot(Tree *const tree, const char *png_file_name);

void TreeDump(Tree *tree, const char *func, const int line);

#endif //TREE_STRUCT_H