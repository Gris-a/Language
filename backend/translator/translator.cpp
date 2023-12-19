#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "translator.h"

static int LABEL_N = 0;

static void TranslateExpression(Node *expr , FILE *asm_file, Function *func_info);
static void TranslateAssignment(Node *assig, FILE *asm_file, Function *func_info);
static void TranslateIf        (Node *_if  , FILE *asm_file, Function *func_info);
static void TranslateBody      (Node *body , FILE *asm_file, Function *func_info);

static void SearchVar(Node *node, Function *func_info);
static Function GetFunctionInfo(Node *func);
static void TranslateFunction  (Node *func , FILE *asm_file);

static void TranslateFuncCall(Node *func , FILE *asm_file, Function *func_info);
static void TranslateAbove   (Node *above, FILE *asm_file, Function *func_info);
static void TranslateAboveEq (Node *abeq , FILE *asm_file, Function *func_info);
static void TranslateLess    (Node *less , FILE *asm_file, Function *func_info);
static void TranslateLessEq  (Node *leeq , FILE *asm_file, Function *func_info);
static void TranslateEq      (Node *eq   , FILE *asm_file, Function *func_info);
static void TranslateNotEq   (Node *neq  , FILE *asm_file, Function *func_info);

static void TranslateAnd(Node *_and, FILE *asm_file, Function *func_info);
static void TranslateOr(Node *_or  , FILE *asm_file, Function *func_info);


static void TranslateAbove(Node *above, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;above_statement\n\n");

    TranslateExpression(above->left , asm_file, func_info);
    fputc('\n', asm_file);
    TranslateExpression(above->right, asm_file, func_info);

    fprintf(asm_file, "ja ABOVE%d\n\n"
                      "push 0\n"
                      "jmp ABOVE_END%d\n\n"
                      "ABOVE%d:\n"
                      "push 1\n\n"
                      "ABOVE_END%d:\n\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateAboveEq(Node *abeq, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;above_eq_statement\n\n");

    TranslateExpression(abeq->left , asm_file, func_info);
    fputc('\n', asm_file);
    TranslateExpression(abeq->right, asm_file, func_info);

    fprintf(asm_file, "jae ABOVE_EQ%d\n\n"
                      "push 0\n"
                      "jmp ABOVE_EQ_END%d\n\n"
                      "ABOVE_EQ%d:\n"
                      "push 1\n\n"
                      "ABOVE_EQ_END%d:\n\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateLess(Node *less, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;below_statement\n\n");

    TranslateExpression(less->left , asm_file, func_info);
    fputc('\n', asm_file);
    TranslateExpression(less->right, asm_file, func_info);

    fprintf(asm_file, "jb BELOW%d\n\n"
                      "push 0\n"
                      "jmp BELOW_END%d\n\n"
                      "BELOW%d:\n"
                      "push 1\n\n"
                      "BELOW_END%d:\n\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateLessEq(Node *leeq, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;less_or_eq_statement\n\n");

    TranslateExpression(leeq->left , asm_file, func_info);
    fputc('\n', asm_file);
    TranslateExpression(leeq->right, asm_file, func_info);

    fprintf(asm_file, "jbe BELOW_EQ%d\n\n"
                      "push 0\n"
                      "jmp BELOW_EQ_END%d\n\n"
                      "BELOW_EQ%d:\n"
                      "push 1\n\n"
                      "BELOW_EQ_END%d:\n\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateEq(Node *eq, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;eq_statement\n\n");

    TranslateExpression(eq->left , asm_file, func_info);
    fputc('\n', asm_file);
    TranslateExpression(eq->right, asm_file, func_info);

    fprintf(asm_file, "je EQ%d\n\n"
                      "push 0\n"
                      "jmp EQ_END%d\n\n"
                      "EQ%d:\n"
                      "push 1\n\n"
                      "EQ_END%d:\n\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateNotEq(Node *neq, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;not_eq_statement\n\n");

    TranslateExpression(neq->left , asm_file, func_info);
    fputc('\n', asm_file);
    TranslateExpression(neq->right, asm_file, func_info);

    fprintf(asm_file, "jne NOT_EQ%d\n\n"
                      "push 0\n"
                      "jmp NOT_EQ_END%d\n\n"
                      "NOT_EQ%d:\n"
                      "push 1\n\n"
                      "NOT_EQ_END%d:\n\n", LABEL_N, LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateAnd(Node *_and, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;and_statement\n\n");

    TranslateExpression(_and->left , asm_file, func_info);
    fprintf(asm_file, "push 0\n"
                      "je FALSE%d\n\n", LABEL_N);

    TranslateExpression(_and->right, asm_file, func_info);
    fprintf(asm_file, "\npush 0\n"
                      "je FALSE%d\n\n", LABEL_N);

    fprintf(asm_file, "push 1\n"
                      "jmp AND_END%d\n\n"
                      "FALSE%d:\n"
                      "push 0\n\n"
                      "AND_END%d:\n\n", LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

static void TranslateOr(Node *_or, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, "\n;or_statement\n\n");

    TranslateExpression(_or->left , asm_file, func_info);
    fprintf(asm_file, "push 0\n"
                      "jne TRUE%d\n\n", LABEL_N);

    TranslateExpression(_or->right, asm_file, func_info);
    fprintf(asm_file, "\npush 0\n"
                      "jne TRUE%d\n\n", LABEL_N);

    fprintf(asm_file, "push 0\n"
                      "jmp OR_END%d\n\n"
                      "TRUE%d:\n"
                      "push 1\n\n"
                      "OR_END%d:\n\n", LABEL_N + 1, LABEL_N, LABEL_N + 1);
    LABEL_N += 2;
}

#define BUILTIN_FUNC(func_name, n_args) if(strcmp(func_name, GetFunction(func)) == 0) {fprintf(asm_file, func_name"\n"); return;} else
static void TranslateFuncCall(Node *func, FILE *asm_file, Function *func_info)
{
    bool is_recursive = false;
    if(func_info->func_name && (strcmp(GetFunction(func), func_info->func_name) == 0))
    {
        is_recursive = true;
        fprintf(asm_file, "\n;local_variables_save\n");
        for(size_t i = 0; i < func_info->n_vars; i++)
        {
            fprintf(asm_file, "push [%d]\n", func_info->variables[i]);
        }
        fputc('\n', asm_file);
    }

    Node *arg = func->left;
    while(arg)
    {
        TranslateExpression(arg->left, asm_file, func_info);
        arg = arg->right;
    }

    #include "../../general/language/functions.h"
    /*else*/ fprintf(asm_file, "call %s\n", func->data.func);

    if(is_recursive)
    {
        fprintf(asm_file, "\n;local_variables_load\n");
        for(size_t i = 0; i < func_info->n_vars; i++)
        {
            fprintf(asm_file, "pop [%d]\n", func_info->variables[i]);
        }
        fputc('\n', asm_file);
    }

    fprintf(asm_file, "push rax\n");
}
#undef BUILTIN_FUNC


static void TranslateExpression(Node *expr, FILE *asm_file, Function *func_info)
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
            TranslateFuncCall(expr, asm_file, func_info);
            break;
        }
        case NodeType::OP:
        {
            switch(GetOperator(expr))
            {
                case Operator::ABOVE:
                {
                    TranslateAbove(expr, asm_file, func_info);
                    break;
                }
                case Operator::ABOVEEQ:
                {
                    TranslateAboveEq(expr, asm_file, func_info);
                    break;
                }
                case Operator::LESS:
                {
                    TranslateLess(expr, asm_file, func_info);
                    break;
                }
                case Operator::LESSEQ:
                {
                    TranslateLessEq(expr, asm_file, func_info);
                    break;
                }
                case Operator::EQ:
                {
                    TranslateEq(expr, asm_file, func_info);
                    break;
                }
                case Operator::NOTEQ:
                {
                    TranslateNotEq(expr, asm_file, func_info);
                    break;
                }
                case Operator::ADD:
                {
                    TranslateExpression(expr->left , asm_file, func_info);
                    TranslateExpression(expr->right, asm_file, func_info);

                    fprintf(asm_file, "add\n");
                    break;
                }
                case Operator::SUB:
                {
                    TranslateExpression(expr->left , asm_file, func_info);
                    TranslateExpression(expr->right, asm_file, func_info);

                    fprintf(asm_file, "sub\n");
                    break;
                }
                case Operator::MUL:
                {
                    TranslateExpression(expr->left , asm_file, func_info);
                    TranslateExpression(expr->right, asm_file, func_info);

                    fprintf(asm_file, "mul\n");
                    break;
                }
                case Operator::DIV:
                {
                    TranslateExpression(expr->left , asm_file, func_info);
                    TranslateExpression(expr->right, asm_file, func_info);

                    fprintf(asm_file, "div\n");
                    break;
                }
                case Operator::POW:
                {
                    TranslateExpression(expr->left , asm_file, func_info);
                    TranslateExpression(expr->right, asm_file, func_info);

                    fprintf(asm_file, "pow\n");
                    break;
                }
                case Operator::AND:
                {
                    TranslateAnd(expr, asm_file, func_info);
                    break;
                }
                case Operator::OR:
                {
                    TranslateOr(expr, asm_file, func_info);
                    break;
                }
                case Operator::ASS:
                case Operator::NOT_OP:
                default: return;
            }
            break;
        }
        case NodeType::SP_CH:
        case NodeType::KWORD:
        case NodeType::WORD:
        case NodeType::NOT_NODE:
        default: return;
    }
}

static void TranslateAssignment(Node *assig, FILE *asm_file, Function *func_info)
{
    fprintf(asm_file, ";assignment\n");
    TranslateExpression(assig->right, asm_file, func_info);
    fprintf(asm_file, "pop [%d]\n\n", (int)GetVariable(assig->left));
}

static void TranslateIf(Node *_if, FILE *asm_file, Function *func_info)
{
    int if_yes = LABEL_N++;
    int if_no  = LABEL_N++;

    fprintf(asm_file, "\n;if_cond\n");
    TranslateExpression(_if->left->right, asm_file, func_info);
    fprintf(asm_file, "push 0\n"
                      "je ELSE%d\n\n", if_no);

    fprintf(asm_file, ";if_body\n");
    TranslateBody(_if->left->left, asm_file, func_info);
    fprintf(asm_file, "jmp IF_END%d\n\n", if_yes);

    fprintf(asm_file, "ELSE%d:\n", if_no);
    fprintf(asm_file, ";else_body\n");
    if(_if->right)
        TranslateBody(_if->right->left, asm_file, func_info);
    fprintf(asm_file, "\nIF_END%d:\n\n", if_yes);
}

static void TranslateWhile(Node *_while, FILE *asm_file, Function *func_info)
{
    int while_begin = LABEL_N++;
    int while_end   = LABEL_N++;

    fprintf(asm_file, "\nWHILE_BEGIN%d:\n\n", while_begin);

    fprintf(asm_file, ";while_cond\n");
    TranslateExpression(_while->left->right, asm_file, func_info);
    fprintf(asm_file, "push 0\n"
                      "je WHILE_END%d\n\n", while_end);

    fprintf(asm_file, ";while_body\n");
    TranslateBody(_while->left->left, asm_file, func_info);
    fprintf(asm_file, "jmp WHILE_BEGIN%d\n\n", while_begin);

    fprintf(asm_file, "WHILE_END%d:\n\n", while_end);
}

static void TranslateBody(Node *body, FILE *asm_file, Function *func_info)
{
    while(body)
    {
        if(IsKeyword(body->left))
        {
            switch(GetKeyword(body->left))
            {
                case Keyword::IF:
                {
                    TranslateIf(body->left, asm_file, func_info);
                    break;
                }
                case Keyword::WHILE:
                {
                    TranslateWhile(body->left, asm_file, func_info);
                    break;
                }
                case Keyword::RET:
                {
                    TranslateExpression(body->left->left, asm_file, func_info);
                    fprintf(asm_file, "\n;ret point\n"
                                      "pop rax\n"
                                      "ret\n\n");
                    break;
                }
                case Keyword::ELSE:
                case Keyword::FUNC:
                case Keyword::NOT_KWORD:
                default: return;
            }
        }
        else if(IsOperator(body->left) && (GetOperator(body->left) == Operator::ASS))
        {
            TranslateAssignment(body->left, asm_file, func_info);
        }
        else
        {
            TranslateExpression(body->left, asm_file, func_info);
        }

        body = body->right;
    }
}


static void SearchVar(Node *node, Function *func_info)
{
    if(!node) return;
    else if(IsVariable(node))
    {
        if(GetVariable(node) >= func_info->var_size)
        {
            func_info->var_size = GetVariable(node) * 2;
            int *temp = (int *)realloc(func_info->variables, sizeof(int) * func_info->var_size);
            if(!temp) return;

            func_info->variables = temp;
        }
        func_info->variables[GetVariable(node)] = true;
    }
    else
    {
        SearchVar(node->left , func_info);
        SearchVar(node->right, func_info);
    }
}

static Function GetFunctionInfo(Node *func)
{
    Function func_info = {.func_name = GetFunction(func),
                          .n_vars    = 0,
                          .var_size  = 256,
                          .variables = (int *)calloc(256, sizeof(int))};
    SearchVar(func, &func_info);

    for(size_t i = 0; i < func_info.var_size; i++)
    {
        if(func_info.variables[i])
        {
            func_info.variables[func_info.n_vars++] = (int)i;
        }
    }
    int *temp = (int *)realloc(func_info.variables, sizeof(int) * (func_info.var_size = func_info.n_vars));
    if(!temp) return {};

    func_info.variables = temp;
    return func_info;
}

static void TranslateFunction(Node *func, FILE *asm_file)
{
    fprintf(asm_file, "\njmp SKIP_FUNC_%s\n\n"
                      "%s:\n", GetFunction(func), GetFunction(func));

    Node *arg = func->right;
    while(arg)
    {
        fprintf(asm_file, "pop [%d]\n", (int)GetVariable(arg->left));
        arg = arg->right;
    }

    Function func_info = GetFunctionInfo(func);
    TranslateBody(func->left, asm_file, &func_info);
    free(func_info.variables);

    fprintf(asm_file, "ret\n");
    fprintf(asm_file, "\nSKIP_FUNC_%s:\n\n", GetFunction(func));
}


void Translator(const char *file_name)
{
    Tree tree = ReadTree(file_name);
    if(!tree.root) return;

    FILE *asm_file = fopen("asm.s", "w");

    Node *parse = tree.root;
    while(parse)
    {
        if(IsKeyword(parse))
        {
            TranslateFunction(parse->left, asm_file);
        }
        else
        {
            TranslateAssignment(parse->left, asm_file, {});
        }
        parse = parse->right;
    }
    fprintf(asm_file, "call main\n"
                      "hlt\n");

    TreeDtor(&tree);

    fclose(asm_file);
}