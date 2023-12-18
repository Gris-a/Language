#include <stdio.h>
#include <string.h>

#include "translator.h"

static int LABEL_N = 0;

static void TranslateExpression(Node *expr , FILE *asm_file);
static void TranslateAssignment(Node *assig, FILE *asm_file);
static void TranslateIf        (Node *_if  , FILE *asm_file);
static void TranslateBody      (Node *body , FILE *asm_file);
static void TranslateFunction  (Node *func , FILE *asm_file);

static void TranslateAbove  (Node *above, FILE *asm_file);
static void TranslateAboveEq(Node *abeq , FILE *asm_file);
static void TranslateLess   (Node *less , FILE *asm_file);
static void TranslateLessEq (Node *leeq , FILE *asm_file);
static void TranslateEq     (Node *eq   , FILE *asm_file);
static void TranslateNotEq  (Node *neq  , FILE *asm_file);

static void TranslateAnd(Node *_and, FILE *asm_file);
static void TranslateOr(Node *_or  , FILE *asm_file);


static void TranslateAbove(Node *above, FILE *asm_file)
{
    TranslateExpression(above->left , asm_file);
    TranslateExpression(above->right, asm_file);

    fprintf(asm_file, "sub\n"
                      "push 0\n"
                      "ja LABEL%d\n"
                      "push 0\n"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 1\n"
                      "LABEL%d:\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateAboveEq(Node *abeq, FILE *asm_file)
{
    TranslateExpression(abeq->left , asm_file);
    TranslateExpression(abeq->right, asm_file);

    fprintf(asm_file, "sub\n"
                      "push 0\n"
                      "jae LABEL%d\n"
                      "push 0\n"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 1\n"
                      "LABEL%d:\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateLess(Node *less, FILE *asm_file)
{
    TranslateExpression(less->left , asm_file);
    TranslateExpression(less->right, asm_file);

    fprintf(asm_file, "sub\n"
                      "push 0\n"
                      "jb LABEL%d\n"
                      "push 0\n"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 1\n"
                      "LABEL%d:\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateLessEq(Node *leeq, FILE *asm_file)
{
    TranslateExpression(leeq->left , asm_file);
    TranslateExpression(leeq->right, asm_file);

    fprintf(asm_file, "sub\n"
                      "push 0\n"
                      "jbe LABEL%d\n"
                      "push 0\n"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 1\n"
                      "LABEL%d:\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateEq(Node *eq, FILE *asm_file)
{
    TranslateExpression(eq->left , asm_file);
    TranslateExpression(eq->right, asm_file);

    fprintf(asm_file, "sub\n"
                      "push 0\n"
                      "je LABEL%d\n"
                      "push 0\n"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 1\n"
                      "LABEL%d:\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateNotEq(Node *neq, FILE *asm_file)
{
    TranslateExpression(neq->left , asm_file);
    TranslateExpression(neq->right, asm_file);

    fprintf(asm_file, "sub\n"
                      "push 0\n"
                      "jne LABEL%d\n"
                      "push 0\n"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 1\n"
                      "LABEL%d:\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}


static void TranslateAnd(Node *_and, FILE *asm_file)
{
    TranslateExpression(_and->left , asm_file);
    fprintf(asm_file, "push 0\n"
                      "je LABEL%d", LABEL_N);
    TranslateExpression(_and->right, asm_file);
    fprintf(asm_file, "push 0\n"
                      "je LABEL%d", LABEL_N);
    fprintf(asm_file, "push 1"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 0\n"
                      "LABEL%d:\n", LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateOr(Node *_or, FILE *asm_file)
{
    TranslateExpression(_or->left , asm_file);
    fprintf(asm_file, "push 0\n"
                      "jne LABEL%d", LABEL_N);
    TranslateExpression(_or->right, asm_file);
    fprintf(asm_file, "push 0\n"
                      "jne LABEL%d", LABEL_N);
    fprintf(asm_file, "push 0\n"
                      "jmp LABEL%d\n"
                      "LABEL%d:\n"
                      "push 1\n"
                      "LABEL%d:\n", LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}


#define BUILTIN_FUNC(func_name, n_args) if(strcmp(func_name, expr->data.func) == 0)\
                                            fprintf(asm_file, func_name"\n"); else
static void TranslateExpression(Node *expr, FILE *asm_file)
{
    switch(expr->type)
    {
        case NodeType::NUM:
        {
            fprintf(asm_file, "push %lg\n", GetNumber(expr));
            return;
        }
        case NodeType::VAR:
        {
            fprintf(asm_file, "push [%d]\n", (int)GetVariable(expr));
            return;
        }
        case NodeType::FUNC:
        {
            Node *arg = expr->left;
            while(arg)
            {
                TranslateExpression(arg->left, asm_file);
                arg = arg->right;
            }

            #include "../../general/language/functions.h"
            /*else*/ fprintf(asm_file, "call %s\n"
                                       "push rax\n", expr->data.func);
            break;
        }
        case NodeType::OP:
        {
            switch(GetOperator(expr))
            {
                case Operator::ABOVE:
                {
                    TranslateAbove(expr, asm_file);
                    break;
                }
                case Operator::ABOVEEQ:
                {
                    TranslateAboveEq(expr, asm_file);
                    break;
                }
                case Operator::LESS:
                {
                    TranslateLess(expr, asm_file);
                    break;
                }
                case Operator::LESSEQ:
                {
                    TranslateLessEq(expr, asm_file);
                    break;
                }
                case Operator::EQ:
                {
                    TranslateEq(expr, asm_file);
                    break;
                }
                case Operator::NOTEQ:
                {
                    TranslateNotEq(expr, asm_file);
                    break;
                }
                case Operator::ADD:
                {
                    TranslateExpression(expr->left , asm_file);
                    TranslateExpression(expr->right, asm_file);

                    fprintf(asm_file, "add\n");
                    break;
                }
                case Operator::SUB:
                {
                    TranslateExpression(expr->left , asm_file);
                    TranslateExpression(expr->right, asm_file);

                    fprintf(asm_file, "sub\n");
                    break;
                }
                case Operator::MUL:
                {
                    TranslateExpression(expr->left , asm_file);
                    TranslateExpression(expr->right, asm_file);

                    fprintf(asm_file, "mul\n");
                    break;
                }
                case Operator::DIV:
                {
                    TranslateExpression(expr->left , asm_file);
                    TranslateExpression(expr->right, asm_file);

                    fprintf(asm_file, "div\n");
                    break;
                }
                case Operator::POW:
                {
                    TranslateExpression(expr->left , asm_file);
                    TranslateExpression(expr->right, asm_file);

                    fprintf(asm_file, "pow\n");
                    break;
                }
                case Operator::AND:
                {
                    TranslateAnd(expr, asm_file);
                    break;
                }
                case Operator::OR:
                {
                    TranslateOr(expr, asm_file);
                    break;
                }
                case Operator::ASS:
                default: return;
            }
            break;
        }
        case NodeType::SP_CH:
        case NodeType::KWORD:
        case NodeType::WORD:
        default: return;
    }
}

static void TranslateAssignment(Node *assig, FILE *asm_file)
{
    TranslateExpression(assig->right, asm_file);
    fprintf(asm_file, "pop [%d]\n", (int)GetVariable(assig->left));
}

static void TranslateIf(Node *_if, FILE *asm_file)
{
    int if_yes = LABEL_N;
    int if_no  = LABEL_N + 1;
    LABEL_N += 2;

    TranslateExpression(_if->left->right, asm_file);
    fprintf(asm_file, "push 0\n"
                      "je LABEL%d\n", if_no);
    TranslateBody(_if->left->left, asm_file);
    fprintf(asm_file, "jmp LABEL%d\n"
                      "LABEL%d:\n", if_yes, if_no);
    if(_if->right)
        TranslateBody(_if->right->left, asm_file);
    fprintf(asm_file, "LABEL%d:\n", if_yes);
}

static void TranslateWhile(Node *_while, FILE *asm_file)
{
    int while_begin = LABEL_N++;
    int while_end   = LABEL_N++;

    fprintf(asm_file, "LABEL%d:\n", while_begin);

    TranslateExpression(_while->left->right, asm_file);
    fprintf(asm_file, "push 0\n"
                      "je LABEL%d\n", while_end);
    TranslateBody(_while->left->left, asm_file);
    fprintf(asm_file, "jmp LABEL%d\n"
                      "LABEL%d:\n", while_begin, while_end);
}

static void TranslateBody(Node *body, FILE *asm_file)
{
    while(body)
    {
        if(IsKeyword(body->left))
        {
            switch(GetKeyword(body->left))
            {
                case Keyword::IF:
                {
                    TranslateIf(body->left, asm_file);
                    break;
                }
                case Keyword::WHILE:
                {
                    TranslateWhile(body->left, asm_file);
                    break;
                }
                case Keyword::RET:
                {
                    TranslateExpression(body->left->left, asm_file);
                    fprintf(asm_file, "pop rax\n"
                                      "ret\n");
                    break;
                }
                case Keyword::FUNC:
                case Keyword::ELSE:
                default: return;
            }
        }
        else if(IsOperator(body->left) && (GetOperator(body->left) == Operator::ASS))
        {
            TranslateAssignment(body->left, asm_file);
        }
        else
        {
            TranslateExpression(body->left, asm_file);
        }

        body = body->right;
    }
}

static void TranslateFunction(Node *func, FILE *asm_file)
{
    int func_end = LABEL_N++;

    fprintf(asm_file, "jmp LABEL%d:\n"
                      "%s:\n", func_end, func->data.func);

    Node *arg = func->right;
    while(arg)
    {
        fprintf(asm_file, "pop [%d]\n", (int)GetVariable(arg->left));
        arg = arg->right;
    }

    TranslateBody(func->left, asm_file);
    fprintf(asm_file, "pop rax\n"
                      "ret\n");

    fprintf(asm_file, "LABEL%d:\n", func_end);
}

void Translator(const char *file_name)
{
    Tree tree = ReadTree(file_name);
    if(!tree.root) return;

    FILE *asm_file = fopen("asm.s", "w");
    fprintf(asm_file, "call main\n"
                      "hlt\n");

    Node *parse = tree.root;
    while(parse)
    {
        if(IsKeyword(parse))
        {
            TranslateFunction(parse->left, asm_file);
        }
        else
        {
            TranslateAssignment(parse->left, asm_file);
        }
        parse = parse->right;
    }

    TreeDtor(&tree);

    fclose(asm_file);
}