#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../include/parser.h"


/*
Expression = '{'{AddSub';'}+'}'
AddSub     = ['-', '+']?MulDiv{['-', '+']MulDiv}*
MulDiv     = Pow{['*', '/']Pow}*
Pow        = Primary{'^'Primary}*
Primary    = Var|Num|Func|Brackets
Var        = ['a' - 'z', 'A' - 'Z']['a' - 'z', 'A' - 'Z', '0' - '9', '_']*
Num        = ['0' - '9']+{'.'['0' - '9']+}?
Func       = [keyword]Brackets
Brackets   = '('AddSub')'
*/

/*
General := {Assignment ';'}*
           {'func' Name '(' FuncArgs ')' '{' FuncBody '}'}*
            'func' main '(' ')' '{' FuncBody '}'
           {'func' Name '(' FuncArgs ')' '{' FuncBody '}'}*

Assignment := Name '=' Expression
Name := ['a' - 'z', 'A' - 'Z']['a' - 'z', 'A' - 'Z', '0' - '9', '_']*
FuncArgs := {Name}?{','Name}*
FuncBody := {Assignment|FuncCall|if|for|while}{return {Name|num}}?
*/

static Tokens Tokenizator(Text *code, NamesTable *lang, NamesTable *global);

static void SyntaxErrMessage(Text *code, size_t line, size_t l_pos);

static bool   IsSpecialCharacter(char ch);
static bool   SkipSpaces        (const char *str, size_t *pos);
static double ScanDouble        (const char *str, size_t *pos);
static void   ScanWord          (const char *str, size_t *pos, char buffer[]);

static Name *SearchNameStack(Stack *stack, const char *name_buf);


static Node *ParseGeneral(Token **token);

static Node *ParseFunction(Token **token);
static Node *ParseFuncArgs(Token **token, size_t *n_args);
static Node *ParseBody(Token **token);
static Node *ParseIf(Token **token);

static Node *ParseAssignment(Token **token);

static Node *ParseExprAddSub(Token **token);
static Node *ParseExprMulDiv(Token **token);
static Node *ParseExprPow   (Token **token);
static Node *ParsePrimary   (Token **token);
static Node *ParseBrackets  (Token **token);
static Node *ParseFuncCall  (Token **token);

static Name *SearchNameStack(Stack *stack, const char *name_buf)
{
    Stack stack_temp = StackCtor();
    NamesTable *table_temp = NULL;
    Name *name             = NULL;

    for(size_t i = stack->size; i > 0; i--)
    {
        table_temp = (NamesTable *)PopStack(stack);
        PushStack(&stack_temp, table_temp);

        name = SearchName(table_temp, name_buf);
        if(name) break;
    }

    for(size_t i = stack_temp.size; i > 0; i--)
    {
        table_temp = (NamesTable *)PopStack(&stack_temp);
        PushStack(stack, table_temp);
    }

    return name;
}

static void SyntaxErrMessage(Text *code, size_t line, size_t l_pos)
{
    printf("Syntax error at line %zu:\n", line);
    printf("%s\n", code->lines[line - 1]);
    printf("%*s\n", (int)l_pos, "^");
}

static void ScanWord(const char *str, size_t *pos, char buffer[])
{
    size_t buf_pos = 0;

    while((isalnum(str[*pos]) || (str[*pos] == '_')) && !(IsSpecialCharacter(str[*pos])))
    {
        buffer[buf_pos++] = str[(*pos)++];
    }
}

static double ScanDouble(const char *str, size_t *pos)
{
    char *end_p   = NULL;
    double result = strtod(str + *pos, &end_p);

    *pos = (size_t)(end_p - str);

    return result;
}

#define SPECIAL_CH(ch) case ch: return true;
static bool IsSpecialCharacter(char ch)
{
    switch(ch)
    {
        #include "../../general/language/special_ch.h"
        default: return false;
    }
}
#undef SPECIAL_CH

static bool SkipSpaces(const char *str, size_t *pos)
{
    while(isspace(str[*pos])) (*pos)++;

    return (str[*pos] == '\0');
}

#define LINE code->lines[line - 1]
#define CHAR code->lines[line - 1][pos]
static Tokens Tokenizator(Text *code, NamesTable *lang, NamesTable *global)
{
    Tokens tokens = TokensCtor();

    Stack nt_stack = StackCtor();
    NamesTable *current = global;

    bool is_end = false;

    size_t line = 1;
    size_t pos  = 0;

    for(; line <= code->n_lines; line++)
    {
        is_end = false;
        pos    = 0;

        while(CHAR != '\0')
        {
            is_end = SkipSpaces(LINE, &pos);
            if(is_end) break;

            size_t old_pos = pos;
            if(isdigit(CHAR))
            {
                double dbl  = ScanDouble(LINE, &pos);
                AddToken(&tokens, {.num = dbl}, NodeType::NUM, line, old_pos);
            }
            else
            {
                char name_buf[BUFSIZ / 16] = {};

                bool is_special = false;
                if((is_special = IsSpecialCharacter(CHAR)))
                {
                    name_buf[0] = CHAR;
                    pos++;

                    if((CHAR == '=') && ((name_buf[0] == '<') ||
                                         (name_buf[0] == '>') ||
                                         (name_buf[0] == '=')))
                    {
                        name_buf[1] = '=';
                    }
                }
                else if((isalpha(CHAR)) || (CHAR == '_'))
                {
                    ScanWord(LINE, &pos, name_buf);
                }
                else
                {
                    SyntaxErrMessage(code, line, old_pos);
                    TokensDtor(&tokens);

                    return {};
                }

                Name *name = SearchName(lang, name_buf);
                if(!name)
                    name = SearchNameStack(&nt_stack, name_buf);
                if(!name)
                    name = SearchName(global, name_buf);

                if(name)
                {
                    AddToken(&tokens, {.name = name}, NodeType::NAME, line, old_pos);
                    if((name->type == NameType::KWORD) && ((name->value.kword == Keyword::FOR)  ||
                                                           (name->value.kword == Keyword::FUNC) ||
                                                           (name->value.kword == Keyword::IF)   ||
                                                           (name->value.kword == Keyword::ELSE) ||
                                                           (name->value.kword == Keyword::WHILE)))
                    {
                        Token *table = AddToken(&tokens, {.table = NamesTableCtor()}, NodeType::TABLE, line, old_pos);
                        current      = table->lexeme->data.table;
                        PushStack(&nt_stack, current);
                    }
                }
                else if(is_special)
                {
                    if(name_buf[0] == '}')
                    {
                        PopStack(&nt_stack);
                        if(nt_stack.size != 0)
                        {
                            current = (NamesTable *)PopStack(&nt_stack);
                            PushStack(&nt_stack, current);
                        }
                        else current = global;

                    }
                    AddToken(&tokens, {.tmp = name_buf[0]}, NodeType::TMP, line, old_pos);
                }
                else
                {
                    Token *last = LastToken(&tokens);
                    last = last->prev;

                    if((IsKeyword(last->lexeme)) && (GetKeyword(last->lexeme) == Keyword::FUNC))
                    {
                        name = AddName(global, name_buf, NameType::FUNC, {.func_n_args = 0});
                    }
                    else
                    {
                        name = AddName(current, name_buf, NameType::VAR, {.var_val = 0});
                    }

                    AddToken(&tokens, {.name = name}, NodeType::NAME, line, old_pos);
                }
            }
        }
    }

    tokens.data->line = line - 1;
    tokens.data->pos  = pos;

    return tokens;
}
#undef LINE
#undef CHAR


Tree ParseCode(const char *file_name)
{
    __attribute__((cleanup(BufDtor)))
    Buffer buf = {};

    __attribute__((cleanup(TextDtor)))
    Text code  = {};

    code = ReadText(file_name, &buf);
    if(!code.lines) return {};

    NamesTable *lang = NamesTableCtorLang();
    NamesTable *global   = NamesTableCtor();

    Tokens tokens = Tokenizator(&code, lang, global);
    if(!tokens.data)
    {
        NamesTableDtor(lang);
        NamesTableDtor(global);

        return {};
    }

    Token *token    = FirstToken(&tokens);
    bool is_syn_err = false;

    Node *root = ParseGeneral(&token, &is_syn_err);
    if(!root || (token != tokens.data))
    {
        SyntaxErrMessage(&code, token->line, token->pos);
        NamesTableDtor(lang);
        NamesTableDtor(global);
        TokensDtor(&tokens);

        return {};
    }

    TokensClear(&tokens);

    Tree tree = {.root   = root,
                 .lang   = lang,
                 .global = global};
    return tree;
}

#define LEXEME (*token)->lexeme
#define TOKEN_NEXT (*token) = (*token)->next
#define TOKEN_PREV (*token) = (*token)->prev
static Node *ParseGeneral(Token **token, bool *is_syn_err)
{
    __attribute__((cleanup(StackDtor)))
    Stack stack = StackCtor();

    Node *expr = NULL;
    while((expr = ParseAssignment(token, is_syn_err)))
    {
        PushStack(&stack, expr);
    }
    if(*is_syn_err) return NULL;

    Node *func = NULL;
    while((func = ParseFunction(token, is_syn_err)))
    {
        PushStack(&stack, func);
    }
    if(*is_syn_err) return NULL;

    if(stack.size == 0)
    {
        *is_syn_err = true;
        return NULL;
    }

    Node *ret_val = (Node *)PopStack(&stack);

    Node *temp = NULL;
    for(size_t i = stack.size; i > 0; i--)
    {
        temp = (Node *)PopStack(&stack);

        temp->right = ret_val;
        ret_val     = temp;
    }

    return ret_val;
}

static Node *ParseFunction(Token **token, bool *is_syn_err)
{
    Node *func = LEXEME;
    if(!(IsKeyword(LEXEME) && GetKeyword(LEXEME) == Keyword::FUNC))
        return NULL;
    TOKEN_NEXT;
    TOKEN_NEXT;

    Node *func_name = LEXEME;
    if(!IsFunction(LEXEME))
    {
        (*is_syn_err) = true;

        return NULL;
    }
    TOKEN_NEXT;

    if(!(IsTemporary (LEXEME) &&
    (GetTemporary(LEXEME) == '(')))
    {
        (*is_syn_err) = true;

        return NULL;
    }
    TOKEN_NEXT;

    Node *args = ParseFuncArgs(token, is_syn_err, &func_name->data.name->value.func_n_args);
    if(*is_syn_err) return NULL;

    if(!(IsTemporary (LEXEME) &&
        (GetTemporary(LEXEME) == ')')))
    {
        (*is_syn_err) = true;

        return NULL;
    }
    TOKEN_NEXT;

    if(!(IsTemporary (LEXEME) &&
    (GetTemporary(LEXEME) == '{')))
    {
        (*is_syn_err) = true;

        return NULL;
    }
    TOKEN_NEXT;

    Node *body = ParseBody(token, is_syn_err);
    if(*is_syn_err) return NULL;

    if(!(IsTemporary (LEXEME) &&
        (GetTemporary(LEXEME) == '}')))
    {
        (*is_syn_err) = true;

        return NULL;
    }
    TOKEN_NEXT;

    func->left = func_name;

    func_name->left  = body;
    func_name->right = args;

    return func;
}

static Node *ParseFuncArgs(Token **token, size_t *n_args)
{
    Node *args = LEXEME;
    if(!IsVariable(args))
        return NULL;
    TOKEN_NEXT;
    (*n_args)++;

    Node *arg_last = args;
    while(IsTemporary(LEXEME) && (GetTemporary(LEXEME) == ','))
    {
        TOKEN_NEXT;
        (*n_args)++;

        Node *arg_next = LEXEME;
        if(!IsVariable(arg_next))
            return NULL;
        TOKEN_NEXT;

        arg_last->left = arg_next;
        arg_last       = arg_next;
    }

    return args;
}

static Node *ParseBody(Token **token)
{
    __attribute__((cleanup(StackDtor)))
    Stack stack = StackCtor();

    while(true)
    {
        Token *current = *token;
        Node *expr     = NULL;

        if(!(expr = ParseAssignment(token)))
        {
            (*token) = current;
            if(!(expr = ParseExprAddSub(token)))
            {
                (*token) = current;
                if(!(expr = ParseIf(token)))
                {
                    break;
                }
            }
        }

        Node *sep = LEXEME;
        if(!(IsOperator(sep) && (GetOperator(sep) == Operator::SEP)))
            return NULL;
        TOKEN_NEXT;

        sep->left = expr;
        PushStack(&stack, sep);
    }

    LOG("FINISHED NO PROBLEM\n");
    if(stack.size == 0)
        return NULL;

    Node *ret_val = (Node *)PopStack(&stack);

    Node *temp = NULL;
    for(size_t i = stack.size; i > 0; i--)
    {
        temp = (Node *)PopStack(&stack);

        temp->right = ret_val;
        ret_val     = temp;
    }

    return ret_val;
}


static Node *ParseIf(Token **token)
{
    Node *_if = LEXEME;
    if(!(IsKeyword(_if) && (GetKeyword(_if) == Keyword::IF)))
        return NULL;
    TOKEN_NEXT;
    TOKEN_NEXT;

    if(!(IsTemporary(LEXEME) && (GetTemporary(LEXEME) == '{')))
        return NULL;
    TOKEN_NEXT;

    _if->left = ParseBody(token);
    if(!_if->left) return NULL;

    if(!(IsTemporary(LEXEME) && (GetTemporary(LEXEME) == '}')))
        return NULL;
    TOKEN_NEXT;

//     Node *_else = LEXEME;
//     if(IsKeyword(_else) && (GetKeyword(_else) == Keyword::ELSE))
//     {
//         TOKEN_NEXT;
//         TOKEN_NEXT;
//
//         if(!(IsTemporary(LEXEME) && (GetTemporary(LEXEME) == '{')))
//             return NULL;
//
//         _else->left = ParseBody(token);
//         if(!_else->left) return NULL;
//
//         if(!(IsTemporary(LEXEME) && (GetTemporary(LEXEME) == '}')))
//             return NULL;
//     }

    // _if->right = _else;

    return _if;
}


static Node *ParseAssignment(Token **token, bool *is_syn_err)
{
    Node *variable = LEXEME;
    if(!IsVariable(variable))
        return NULL;
    TOKEN_NEXT;

    Node *ass = LEXEME;
    if(!(IsOperator(ass) && (GetOperator(ass) == Operator::ASS)))
        return NULL;
    TOKEN_NEXT;

    Node *expr = ParseExprAddSub(token, is_syn_err);
    if(*is_syn_err) return NULL;

    Node *sep = LEXEME;
    if(!(IsOperator(sep) && (GetOperator(sep) == Operator::SEP)))
    {
        (*is_syn_err) = true;

        return NULL;
    }
    TOKEN_NEXT;

    sep->left  = ass;
    ass->left  = variable;
    ass->right = expr;

    return sep;
}







static Node *ParseExprAddSub(Token **token)
{
    Node *expr = NULL;
    Node *op   = LEXEME;

    if(IsOperator(op) && ((GetOperator(op) == Operator::ADD) ||
                          (GetOperator(op) == Operator::SUB)))
    {
        InsertToken((*token), {.num = 0}, NodeType::NUM, (*token)->line, (*token)->pos);
        TOKEN_NEXT;
        expr     = LEXEME;
    }
    else
    {
        expr = ParseExprMulDiv(token);
        if(!expr) return NULL;

        op = LEXEME;
    }

    Node *ret_val = expr;

    while(IsOperator(op) && ((GetOperator(op) == Operator::ADD) ||
                             (GetOperator(op) == Operator::SUB)))
    {
        TOKEN_NEXT;

        expr = ParseExprMulDiv(token);
        if(!expr) return NULL;

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprMulDiv(Token **token)
{
    Node *expr = ParseExprPow(token);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op      = LEXEME;

    while(IsOperator(op) && ((GetOperator(op) == Operator::MUL) ||
                             (GetOperator(op) == Operator::DIV)))
    {
        TOKEN_NEXT;

        expr = ParseExprPow(token);
        if(!expr) return NULL;

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprPow(Token **token)
{
    Node *expr = ParsePrimary(token);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op      = LEXEME;

    while(IsOperator(op) && (GetOperator(op) == Operator::POW))
    {
        TOKEN_NEXT;

        expr = ParsePrimary(token);
        if(!expr) return NULL;

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParsePrimary(Token **token)
{
    Node *ret_val = LEXEME;

    switch(ret_val->type)
    {
        case NodeType::NUM:  TOKEN_NEXT; break;
        case NodeType::NAME:
        {
            if((ret_val->data.name->type == NameType::KWORD) || (ret_val->data.name->type == NameType::OP))
                return NULL;
            else if(ret_val->data.name->type == NameType::FUNC)
                ret_val = ParseFuncCall(token);
            else
                TOKEN_NEXT;
            break;
        }
        case NodeType::TMP:
        {
            ret_val = ParseBrackets(token);
            break;
        }
        case NodeType::TABLE:
        default: return NULL;
    }

    return ret_val;
}

static Node *ParseFuncCall(Token **token)
{
    return NULL;
}

static Node *ParseBrackets(Token **token)
{
    if(!(IsTemporary (LEXEME) && (GetTemporary(LEXEME) == '(')))
    {
        return NULL;
    }
    TOKEN_NEXT;

    Node *ret_val = ParseExprAddSub(token);

    if(!(IsTemporary (LEXEME) && (GetTemporary(LEXEME) == ')')))
    {
        return NULL;
    }
    TOKEN_NEXT;

    return ret_val;
}