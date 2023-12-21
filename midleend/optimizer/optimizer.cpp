#include <stdio.h>
#include <math.h>

#include "optimizer.h"

static void SubTreeOpt(Node **node, bool *changes);

static double FastPow(double base, long long power);
static bool IsSubTreeCalculated(Node *node);
static double SubTreeCalculate(Node *node);

#define OPERATOR(enum, kword, optimization, ...) case Operator::enum: optimization
static void SubTreeOpt(Node **node, bool *changes)
{
    if(!(*node)) return;

    switch((*node)->type)
    {
        case NodeType::KWORD:
        case NodeType::FUNC:
        case NodeType::SP_CH:
        {
            SubTreeOpt(&(*node)->left , changes);
            SubTreeOpt(&(*node)->right, changes);
            break;
        }
        case NodeType::OP:
        {
            Node *copy = NULL;
            if(IsSubTreeCalculated(*node))
            {
                copy = NodeCtor({.num = SubTreeCalculate(*node)}, NodeType::NUM);
            }
            else
            {
                switch(GetOperator(*node))
                {
                    #include "../../general/language/operators.h"
                    case Operator::NOT_OP:
                    default: return;
                }
            }

            if(copy)
            {
                *changes = true;

                Tree temp = {.root = (*node)};
                TreeDtor(&temp);

                *node = copy;
                SubTreeOpt(node, changes);
            }
            else
            {
                SubTreeOpt(&(*node)->left , changes);
                SubTreeOpt(&(*node)->right, changes);
            }
            break;
        }
        case NodeType::NUM:
        case NodeType::VAR:
        case NodeType::WORD:
        case NodeType::NOT_NODE:
        default: break;
    }

    return;
}
#undef OPERATOR

void Optimizer(const char *in, const char *out)
{
    Tree tree = ReadTree(in);
    if(!tree.root) return;

    bool changes = false;
    while(true)
    {
        changes = false;
        SubTreeOpt(&tree.root, &changes);
        if(!changes) break;
    }

    TreeDot(&tree, "tree_opt.png");

    SaveTree(&tree, out);
    TreeDtor(&tree);
}


static double FastPow(double base, long long power)
{
    double result = 1;
    while(power > 0)
    {
        if(power % 2 == 1)
        {
            result *= base;
        }

        base  *= base;
        power /= 2;
    }

    return result;
}

static bool IsSubTreeCalculated(Node *node)
{
    if(!node) return true;

    switch(node->type)
    {
        case NodeType::OP:
        {
            if(GetOperator(node) == Operator::ASS)
                return false;
            else
            {
                return (IsSubTreeCalculated(node->left) && IsSubTreeCalculated(node->right));
            }
        }
        case NodeType::NUM: return true;
        case NodeType::FUNC:
        case NodeType::KWORD:
        case NodeType::SP_CH:
        case NodeType::WORD:
        case NodeType::VAR:
        case NodeType::NOT_NODE:
        default: return false;
    }
    return false;
}

#define OPERATOR(enum, kword, opt, calculate) case Operator::enum: calculate
static double SubTreeCalculate(Node *node)
{
    if(!node) return NAN;

    switch(node->type)
    {
        case NodeType::OP:
        {
            double lvalue = SubTreeCalculate(node->left);
            double rvalue = SubTreeCalculate(node->right);

            switch(GetOperator(node))
            {
                #include "../../general/language/operators.h"
                case Operator::NOT_OP:
                default:
                {
                    printf("Error: Invalid operator.\n");
                    return NAN;
                }
            }
        }
        case NodeType::NUM:
        {
            return GetNumber(node);
        }
        case NodeType::SP_CH:
        case NodeType::VAR:
        case NodeType::KWORD:
        case NodeType::WORD:
        case NodeType::FUNC:
        case NodeType::NOT_NODE:
        default: return NAN;
    }

    return NAN;
}
#undef OPERATOR