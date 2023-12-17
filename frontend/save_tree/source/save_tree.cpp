#include <stdio.h>

#include "../include/save_tree.h"

static void NodeDataPrint(Node *node, FILE *file);
static void SaveSubTree  (Node *node, FILE *file);

static void NodeDataPrint(Node *node, FILE *file)
{
    switch(node->type)
    {
        case NodeType::NUM:
        {
            fprintf(file, "|%lg|", node->data.num);
            return;
        }
        case NodeType::FUNC:
        {
            fprintf(file, "\"%s\"", node->data.func);
            return;
        }
        case NodeType::KWORD:
        {
            fprintf(file, "[%d]", GetKeyword(node));
            return;
        }
        case NodeType::OP:
        {
            fprintf(file, "{%d}", GetOperator(node));
            return;
        }
        case NodeType::VAR:
        {
            fprintf(file, "<%zu>", GetVariable(node));
            return;
        }
        case NodeType::SP_CH:
        {
            fprintf(file, "\'%d\'", GetSpecialChar(node));
            return;
        }
        case NodeType::WORD:
        default:
        {
            fprintf(file, "<<<What the fuck is this?>>>");
            return;
        }
    }
}

static void SaveSubTree(Node *node, FILE *file)
{
    if(!node) {fprintf(file, "*"); return;}

    NodeDataPrint(node, file);

    fprintf(file, "(");
    SaveSubTree(node->left , file);
    SaveSubTree(node->right, file);
    fprintf(file, ")");
}

void SaveTree(Tree *tree, FILE *file)
{
    SaveSubTree(tree->root, file);
}
