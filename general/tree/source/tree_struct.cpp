#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <sys/stat.h>

#include "../include/tree.h"

static void NodeDataPrint(Node *node, FILE *file);

static void SubTreeLog(Node *node, int n_tabs = 0);

static void DotSubTreeCtor(Node *node, Node *node_next, const char *direction, FILE *dot_file);
static void DotTreeGeneral(Tree *tree, FILE *dot_file);

static void MakeDumpDir(void);

static NodeType ReadData(char **buf, node_t *data);
static Node *ReadSubTree(char *buf);
static size_t FileSize(const char *file_name);


static NodeType ReadData(char **buf, node_t *data)
{
    int offset = 0;
    char ch = '\0';

    sscanf(*buf, "%c%n", &ch, &offset);
    *buf += offset;

    switch(ch)
    {
        case '\'':
        {
            int ch_code = 0;
            sscanf(*buf, "%d%n", &ch_code, &offset);
            *buf += offset;

            if(*((*buf)++) != '\'')
            {
                printf("Error: Invalid data.\n");

                break;
            }

            *data = {.sp_ch = (SpecialChar)ch_code};
            return NodeType::SP_CH;
        }
        case '\"':
        {
            char func_name_buf[BUFSIZ / 16] = {};
            sscanf(*buf, "%[^\"]%n", func_name_buf, &offset);
            *buf += offset;

            if(*((*buf)++) != '\"')
            {
                printf("Error: Invalid data.\n");

                break;
            }

            *data = {.func = strdup(func_name_buf)};
            return NodeType::FUNC;
        }
        case '|':
        {
            double num = 0;
            sscanf(*buf, "%lg%n", &num, &offset);
            *buf += offset;

            if(*((*buf)++) != '|')
            {
                printf("Error: Invalid data.\n");

                break;
            }

            *data = {.num = num};
            return NodeType::NUM;
        }
        case '[':
        {
            int kword_code = 0;
            sscanf(*buf, "%d%n", &kword_code, &offset);
            *buf += offset;

            if(*((*buf)++) != ']')
            {
                printf("Error: Invalid data.\n");

                break;
            }

            *data = {.kword = (Keyword)kword_code};
            return NodeType::KWORD;
        }
        case '{':
        {
            int op_code = 0;
            sscanf(*buf, "%d%n", &op_code, &offset);
            *buf += offset;

            if(*((*buf)++) != '}')
            {
                printf("Error: Invalid data.\n");

                break;
            }

            *data = {.op = (Operator)op_code};
            return NodeType::OP;
        }
        case '<':
        {
            size_t var_id = 0;
            sscanf(*buf, "%zu%n", &var_id, &offset);
            *buf += offset;

            if(*((*buf)++) != '>')
            {
                printf("Error: Invalid data.\n");

                break;
            }

            *data = {.var_id = var_id};
            return NodeType::VAR;
        }
        default: break;
    }

    return NodeType::NOT_NODE;
}

static Node *ReadSubTree(char *const buffer)
{
    static char *buf = buffer;
    int offset = 0;
    char ch    = 0;

    sscanf(buf, "%c%n", &ch, &offset);
    buf += offset;

    switch(ch)
    {
        case '(':
        {
            node_t data   = {};
            NodeType type = ReadData(&buf, &data);
            if(type == NodeType::NOT_NODE)
            {
                printf("Error: Invalid data.\n");

                return NULL;
            }

            Node *left  = ReadSubTree(buf);
            Node *right = ReadSubTree(buf);

            if(*(buf++) != ')')
            {
                printf("Error: Invalid data.\n");
                NodeDtor(left );
                NodeDtor(right);

                return NULL;
            }

            return NodeCtor(data, type, left, right);
        }
        case '*':
        {
            return NULL;
        }
        default:
        {
            printf("Error: Invalid data\n");
            return NULL;
        }
    }
}

static size_t FileSize(const char *file_name)
{
    struct stat file_info = {};
    stat(file_name, &file_info);

    return (size_t)file_info.st_size;
}

Tree ReadTree(const char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    if(!file)
    {
        printf("Error: No such file: \"%s\"", file_name);

        return {};
    }

    size_t buf_size = FileSize(file_name);
    char *buffer = (char *)calloc(buf_size + 1, sizeof(char));
    if(!buffer)
    {
        LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);
        fclose(file);

        return {};
    }

    fread(buffer, buf_size, sizeof(char), file);
    fclose(file);

    Tree tree = {};
    tree.root = ReadSubTree(buffer);

    free(buffer);

    return tree;
}

Node *SubTreeCopy(Node *node)
{
    if(!node) return NULL;
    return NodeCtor(node->data, node->type, SubTreeCopy(node->left),
                                            SubTreeCopy(node->right));
}


void SubTreeDtor(Node *sub_tree)
{
    if(!sub_tree) return;

    SubTreeDtor(sub_tree->left);
    SubTreeDtor(sub_tree->right);

    NodeDtor(sub_tree);
}

int TreeDtor(Tree *tree)
{
    ASSERT(tree && tree->root, return EXIT_FAILURE);

    SubTreeDtor(tree->root);

    tree->root = NULL;

    return EXIT_SUCCESS;
}


Node *NodeCtor(node_t val, NodeType type, Node *left, Node *right)
{
    Node *node = (Node *)calloc(1, sizeof(Node));
    if(!node)
    {
        LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);

        return NULL;
    }

    node->type  = type;
    node->data  = val;

    node->left  = left;
    node->right = right;

    return node;
}

int NodeDtor(Node *node)
{
    ASSERT(node, return EXIT_FAILURE);

    switch(node->type)
    {
        case NodeType::FUNC:
        {
            free(GetFunction(node));
            break;
        }
        case NodeType::WORD:
        {
            free(GetWord(node));
            break;
        }
        case NodeType::KWORD:
        case NodeType::OP:
        case NodeType::NUM:
        case NodeType::VAR:
        case NodeType::SP_CH:
        case NodeType::NOT_NODE:
        default: break;
    }

    free(node);

    return EXIT_SUCCESS;
}


#define SPECIAL_CH(enum, ch) case SpecialChar::enum: {fprintf(file, "\\%c", ch); break;}
#define KEYWORD(enum, kword) case Keyword::enum: {fprintf(file, "%s", kword); break;}
#define OPERATOR(enum, op, ...) case Operator::enum: {fprintf(file, "\\%s", op); break;}
static void NodeDataPrint(Node *node, FILE *file)
{
    switch(node->type)
    {
        case NodeType::NUM:
        {
            fprintf(file, "%lg", node->data.num);
            return;
        }
        case NodeType::FUNC:
        {
            fprintf(file, "\\%s", GetFunction(node));
            return;
        }
        case NodeType::KWORD:
        {
            switch(GetKeyword(node))
            {
                #include "../../language/keywords.h"
                case Keyword::NOT_KWORD:
                default:
                    fprintf(file, "<<<What the fuck is this?>>>");
            }
            return;
        }
        case NodeType::OP:
        {
            switch(GetOperator(node))
            {
                #include "../../language/operators.h"
                case Operator::NOT_OP:
                default:
                    fprintf(file, "<<<What the fuck is this?>>>");
            }
            return;
        }
        case NodeType::VAR:
        {
            fprintf(file, "%zu", GetVariable(node));
            return;
        }
        case NodeType::WORD:
        {
            fprintf(file, "\\%s", GetWord(node));
            return;
        }
        case NodeType::SP_CH:
        {
            switch(GetSpecialChar(node))
            {
                #include "../../language/special_ch.h"
                case SpecialChar::NOT_SP_CH:
                default:
                    fprintf(file, "<<<What the fuck is this?>>>");
            }
            return;
        }
        case NodeType::NOT_NODE:
        default:
        {
            fprintf(file, "<<<What the fuck is this?>>>");
            return;
        }
    }
}
#undef SPECIAL_CH
#undef KEYWORD
#undef OPERATOR


static void SubTreeLog(Node *node, int n_tabs)
{
    if(!node) {LOG("\n%*snil\n", n_tabs * 4, ""); return;}

    LOG("\n%*s", n_tabs * 4, "");
    NodeDataPrint(node, LOG_FILE);

    LOG("\n%*s(\n", n_tabs * 4, "");
    SubTreeLog(node->left , n_tabs + 1);
    SubTreeLog(node->right, n_tabs + 1);
    LOG("\n%*s)\n", n_tabs * 4, "");
}

void TreeLog(Tree *tree)
{
    LOG_START;

    LOG("TREE[%p]:\n", tree);
    if(!tree) return;

    LOG("\troot: %p;\n", tree->root);
    if(!tree->root) return;

    SubTreeLog(tree->root);

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
        case NodeType::FUNC:
        {
            fprintf(dot_file , "\"bisque\"];");
            break;
        }
        case NodeType::KWORD:
        {
            fprintf(dot_file, "\"crimson\"];");
            break;
        }
        case NodeType::OP:
        {
            fprintf(dot_file, "\"deeppink3\"];");
            break;
        }
        case NodeType::VAR:
        {
            fprintf(dot_file, "\"gold\"];");
            break;
        }
        case NodeType::SP_CH:
        {
            fprintf(dot_file, "\"green\"];");
            break;
        }
        case NodeType::WORD:
        {
            fprintf(dot_file, "\"white\"];");
            break;
        }
        case NodeType::NOT_NODE:
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

    DotSubTreeCtor(node_next, node_next->left , "left", dot_file);
    DotSubTreeCtor(node_next, node_next->right, "right", dot_file);
}

static void DotTreeGeneral(Tree *tree, FILE *dot_file)
{
    fprintf(dot_file, "digraph\n"
                      "{\n"
                      "bgcolor = \"grey\";\n"
                      "node[shape = \"Mrecord\"; style = \"filled\"; fillcolor = \"#58CD36\"];\n");

    fprintf(dot_file, "{rank = source;");
    DotNodeCtor(tree->root, dot_file);
    fprintf(dot_file, "};\n");
}

void TreeDot(Tree *const tree, const char *png_file_name)
{
    if(!(tree && tree->root)) return;

    FILE *dot_file = fopen("tree.dot", "wb");

    DotTreeGeneral(tree, dot_file);
    DotSubTreeCtor(tree->root, tree->root->left , "left", dot_file);
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