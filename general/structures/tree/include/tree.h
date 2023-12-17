#ifndef TREE_H
#define TREE_H

#include <stdbool.h>

#include "../../../log/log.h"
#include "../types/include/types.h"
#include "names.h"

#define TREE_DUMP(tree_ptr) LOG("Called from %s:%s:%d:\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                            TreeDump(tree_ptr, __func__, __LINE__);

#ifdef PROTECT
#define TREE_VERIFICATION(tree_ptr, ret_val_on_fail) if(!IsTreeValid(tree_ptr))\
                                                     {\
                                                         LOG("%s:%s:%d: Error: invalid tree.\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                                                         TREE_DUMP(tree_ptr);\
                                                         return ret_val_on_fail;\
                                                     }
#else
#define TREE_VERIFICATION(tree_ptr, ...)
#endif

void SubTreeDtor(Node *sub_tree);

int TreeDtor(Tree *tree, Node **root);

Node *NodeCtor(node_t val, NodeType type, Node *left = NULL, Node *right = NULL);

int NodeDtor(Node *node);

void TreeLog(Tree *tree, FILE *file = LOG_FILE);

void TreeDot(Tree *const tree, const char *png_file_name = "tree.png");

void TreeDump(Tree *tree, const char *func, const int line);

#ifdef PROTECT
bool IsTreeValid(Tree *const tree);
#endif

#endif //TREE_H