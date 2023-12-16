#include <stdio.h>
#include <stdlib.h>

#include "../include/tree.h"

static void NodeDataPrint(Node *node, FILE *file = LOG_FILE);

static void SubTreeLog(Node *node, FILE *file, int n_tabs = 0);

static void DotSubTreeCtor(Node *node, Node *node_next, const char *direction, FILE *dot_file);
static void DotTreeGeneral(Tree *tree, FILE *dot_file);

static void MakeDumpDir(void);


void SubTreeDtor(Node *sub_tree)
{
    if(!sub_tree) return;

    SubTreeDtor(sub_tree->left);
    SubTreeDtor(sub_tree->right);

    NodeDtor(sub_tree);
}

int TreeDtor(Tree *tree, Node **root)
{
    TREE_VERIFICATION(tree, EXIT_FAILURE);
    ASSERT(root && (*root), return EXIT_FAILURE);

    SubTreeDtor(*root);

    (*root) = NULL;

    return EXIT_SUCCESS;
}


Node *NodeCtor(node_t val, NodeType type, Node *left, Node *right)
{
    Node *node = (Node *)calloc(1, sizeof(Node));

    node->type  = type;
    node->data  = val;

    node->left  = left;
    node->right = right;

    return node;
}

int NodeDtor(Node *node)
{
    ASSERT(node, return EXIT_FAILURE);

    if(node->type == NodeType::WORD)
        free(node->data.word);
    free(node);

    return EXIT_SUCCESS;
}

#define SPECIAL_CH(enum, ch) case SpecialChar::enum: {fprintf(file, "\\%c", ch); break;}
static void NodeDataPrint(Node *node, FILE *file)
{
    switch(node->type)
    {
        case NodeType::NUM:
        {
            fprintf(file, "%lg", node->data.num);
            return;
        }
        case NodeType::NAME:
        {
            fprintf(file, "\\%s", node->data.name->ident);
            return;
        }
        case NodeType::WORD:
        {
            fprintf(file, "\\%s", node->data.word);
            return;
        }
        case NodeType::SP_CH:
        {
            switch(GetSpecialChar(node))
            {
                #include "../../../general/language/special_ch.h"
                default:
                    fprintf(file, "<<<What the fuck is this?>>>");
            }

            return;
        }
        default:
        {
            fprintf(file, "<<<What the fuck is this?>>>");
            return;
        }
    }
}

static void SubTreeLog(Node *node, FILE *file, int n_tabs)
{
    if(!node) {LOG("\n%*snil\n", n_tabs * 4, ""); return;}

    LOG("\n%*s", n_tabs * 4, "");
    NodeDataPrint(node, file);

    LOG("\n%*s(\n", n_tabs * 4, "");
    SubTreeLog(node->left , file, n_tabs + 1);
    SubTreeLog(node->right, file, n_tabs + 1);
    LOG("\n%*s)\n", n_tabs * 4, "");
}

void TreeLog(Tree *tree, FILE *file)
{
    LOG_START;

    LOG("TREE[%p]:\n", tree);
    if(!tree) return;

    LOG("\troot: %p;\n", tree->root);
    if(!tree->root) return;

    SubTreeLog(tree->root, file);

    LOG_END;
}



static void DotNodeCtor(Node *const node, FILE *dot_file)
{
    fprintf(dot_file, "node%p[label = \"{<data>", node);
    NodeDataPrint(node, dot_file);

    fprintf(dot_file, "| {<left> left | <right> right}}\"; fillcolor = ");

    switch(node->type)
    {
        case NodeType::NUM:
        {
            fprintf(dot_file , "\"coral\"];");
            break;
        }
        case NodeType::NAME:
        {
            fprintf(dot_file , "\"bisque\"];");
            break;
        }
        case NodeType::SP_CH:
        {
            fprintf(dot_file, "\"green\"];");
            break;
        }
        case NodeType::WORD: //fall through
        default:
        {
            fprintf(dot_file , "\"red\"];");
        }
    }
}

static void DotSubTreeCtor(Node *node, Node *node_next, const char *direction, FILE *dot_file)
{
    if(!node_next) return;

    DotNodeCtor(node_next, dot_file);

    fprintf(dot_file, "node%p:<%s>:s -> node%p:<data>:n;\n", node, direction, node_next);

    DotSubTreeCtor(node_next, node_next->left , "left" , dot_file);
    DotSubTreeCtor(node_next, node_next->right, "right", dot_file);
}

static void DotTreeGeneral(Tree *tree, FILE *dot_file)
{
    fprintf(dot_file, "digraph\n"
                      "{\n"
                      "bgcolor = \"grey\";\n"
                      "node[shape = \"Mrecord\"; style = \"filled\"; fillcolor = \"#58CD36\"];\n");

    fprintf(dot_file, "{rank = source;");
    fprintf(dot_file, "nodel[label = \"<root> root: %p\"; fillcolor = \"lightblue\"];",
                                                                           tree->root);
    DotNodeCtor(tree->root, dot_file);

    fprintf(dot_file, "};\n");
}

void TreeDot(Tree *const tree, const char *png_file_name)
{
    if(!(tree && tree->root)) return;

    FILE *dot_file = fopen("tree.dot", "wb");

    DotTreeGeneral(tree, dot_file);
    DotSubTreeCtor(tree->root, tree->root->left , "left" , dot_file);
    DotSubTreeCtor(tree->root, tree->root->right, "right", dot_file);

    fprintf(dot_file, "}\n");

    fclose(dot_file);

    char *sys_cmd = NULL;
    asprintf(&sys_cmd, "dot tree.dot -T png -o %s", png_file_name);
    system(sys_cmd);
    free(sys_cmd);

    remove("tree.dot");
}



static void MakeDumpDir(void)
{
    system("rm -rf dump_tree");
    system("mkdir dump_tree");
}

void TreeDump(Tree *tree, const char *func, const int line)
{
    static int num = 0;

    if(num == 0) MakeDumpDir();

    TreeLog(tree);

    char *file_name = NULL;

    asprintf(&file_name, "dump_tree/tree_dump%d__%s:%d__.png", num, func, line);
    TreeDot(tree, file_name);
    free(file_name);

    num++;
}

#ifdef PROTECT
//TODO upgrade
bool IsTreeValid(Tree *tree)
{
    ASSERT(tree && tree->root, return false);

    return true;
}
#endif