#include <stdio.h>
#include <math.h>

#include "codegen.h"

static void Tabs(FILE *file, int n_tabs);
static void CodeGenBody(Node *body, FILE *code, int n_tabs);
static void CodeGenFuncArgs(Node *func_args, FILE *code);
static void CodeGenFunction(Node *func, FILE *code);
static void CodeGenExpression(Node *expr, FILE *code);
static void CodeGenIf(Node *_if, FILE *code, int n_tabs);
static void CodeGenWhile(Node *_while, FILE *code, int n_tabs);
static void CodeGenReturn(Node *ret, FILE *code, int n_tabs);
static void CodeGenAssignment(Node *ass, FILE *code, int n_tabs);

static void Tabs(FILE *file, int n_tabs)
{
    fprintf(file, "%*s", n_tabs * 4, "");
}

#define OPERATOR(enum, kword, ...) case Operator::enum: fprintf(code, " %s ", kword); break;
static void CodeGenExpression(Node *expr, FILE *code)
{
    if(IsNumber(expr))
    {
        if(GetNumber(expr) < 0)
            fprintf(code, "minus %lg", -GetNumber(expr));
        else
            fprintf(code, "%lg", GetNumber(expr));
    }
    else if(IsVariable(expr))
        fprintf(code, "x%zu", GetVariable(expr));
    else if(IsFunction(expr))
    {
        fprintf(code, "%s", GetFunction(expr));
        CodeGenFuncArgs(expr->left, code);
    }
    else if(IsOperator(expr))
    {
        fputc('}', code);
        if(!(((GetOperator(expr) == Operator::ADD) || (GetOperator(expr) == Operator::SUB)) && IsNumber(expr->left) && (abs(GetNumber(expr->left)) < M_ERR)))
            CodeGenExpression(expr->left, code);
        switch(GetOperator(expr))
        {
            #include "../../general/language/operators.h"
            case Operator::NOT_OP:
            default: break;
        }
        CodeGenExpression(expr->right, code);
        fputc('{', code);
    }
}

static void CodeGenIf(Node *_if, FILE *code, int n_tabs)
{
    Tabs(code, n_tabs);
    fprintf(code, "esli");

    fputc('}', code);
    CodeGenExpression(_if->left->right, code);
    fputc('{' , code);
    fputc('\n', code);
    CodeGenBody(_if->left->left, code, n_tabs);

    if(_if->right)
    {
        Tabs(code, n_tabs);
        fprintf(code, "inache\n");
        CodeGenBody(_if->right->left, code, n_tabs);
    }
}

static void CodeGenWhile(Node *_while, FILE *code, int n_tabs)
{
    Tabs(code, n_tabs);
    fprintf(code, "poka");

    fputc('}', code);
    CodeGenExpression(_while->right, code);
    fputc('{' , code);
    fputc('\n', code);
    CodeGenBody(_while->left, code, n_tabs);
}

static void CodeGenReturn(Node *ret, FILE *code, int n_tabs)
{
    Tabs(code, n_tabs);
    fprintf(code, "VoZdat ");
    CodeGenExpression(ret->left, code);
    fputc('$' , code);
    fputc('\n', code);
}

static void CodeGenBody(Node *body, FILE *code, int n_tabs)
{
    Tabs(code, n_tabs);
    fputc(')' , code);
    fputc('\n', code);

    while(body)
    {
        if(IsOperator(body->left) && (GetOperator(body->left) == Operator::ASS))
        {
            CodeGenAssignment(body->left, code, n_tabs + 1);
        }
        else if(IsKeyword(body->left))
        {
            switch(GetKeyword(body->left))
            {
                case Keyword::IF:
                    CodeGenIf(body->left, code, n_tabs + 1);
                    break;
                case Keyword::WHILE:
                    CodeGenWhile(body->left, code, n_tabs + 1);
                    break;
                case Keyword::RET:
                    CodeGenReturn(body->left, code, n_tabs + 1);
                    break;
                case Keyword::FUNC:
                case Keyword::ELSE:
                case Keyword::NOT_KWORD:
                default: break;
            }
        }
        else
        {
            Tabs(code, n_tabs + 1);
            CodeGenExpression(body->left, code);
            fputc('$', code);
            fputc('\n', code);
        }

        body = body->right;
    }

    Tabs(code, n_tabs);
    fputc('(' , code);
    fputc('\n', code);
}

static void CodeGenFuncArgs(Node *func_args, FILE *code)
{
    fputc('}', code);

    if(func_args)
    {
        CodeGenExpression(func_args->left, code);
        func_args = func_args->right;
    }

    while(func_args)
    {
        fputc(',', code);
        fputc(' ', code);
        CodeGenExpression(func_args->left, code);

        func_args = func_args->right;
    }

    fputc('{', code);
}

static void CodeGenFunction(Node *func, FILE *code)
{
    fprintf(code, "func %s", GetFunction(func));

    CodeGenFuncArgs(func->right, code);
    fputc('\n', code);
    CodeGenBody(func->left, code, 1);
}

static void CodeGenAssignment(Node *ass, FILE *code, int n_tabs)
{
    Tabs(code, n_tabs);
    fprintf(code, "x%zu", GetVariable(ass->left));
    fprintf(code, " raVno ");
    CodeGenExpression(ass->right, code);
    fputc('$', code);
    fputc('\n', code);
}

void CodeGen(const char *in, const char *out)
{
    Tree tree = ReadTree(in);
    if(!tree.root) return;

    FILE *code = fopen(out, "w");

    Node *current = tree.root;
    while(current)
    {
        if(IsSpecialChar(current))
            CodeGenAssignment(current->left, code, 0);
        else
            CodeGenFunction(current->left, code);
        current = current->right;
    }

    fclose(code);

    TreeDtor(&tree);
}