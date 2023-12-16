#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../include/parser.h"

static NamesTable *LANG = NamesTableCtorLang();

static NamesTable *TABLE_OF_TABLES[100] = {LANG};
static size_t TABLE_POS = 1;

void clear_tables(void)
{
    for(size_t i = 0; i < TABLE_POS; i++)
    {
        NamesTableDtor(TABLE_OF_TABLES[i]);
    }
}





static Name *SearchNameStackTyped(Stack *stack, const char *name_buf, NameType type);

static void SyntaxErrMessage(Text *code, size_t line, size_t l_pos);

static bool IsSpecialCharacter(char ch);

static bool   SkipSpaces(const char *str, size_t *pos);
static double ScanDouble(const char *str, size_t *pos);
static int    ScanWord  (const char *str, size_t *pos, char buffer[]);

static Tokens Tokenizator(Text *code);

static Node *ParseGeneral(Token **token, bool *is_syn_err);

static Node *ParseFunction(Stack *nt_stack  , Token **token, bool *is_syn_err);
static Node *ParseFuncArgs(NamesTable *local, Token **token, bool *is_syn_err, size_t *n_args);

static Node *ParseIf   (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseWhile(Stack *nt_stack, Token **token, bool *is_syn_err);

static Node *ParseBody      (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseAssignment(Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseReturn    (Stack *nt_stack, Token **token, bool *is_syn_err);


static Node *ParseExpression  (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprAddSub  (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprMulDiv  (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprPow     (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprPrimary (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprBrackets(Stack *nt_stack, Token **token, bool *is_syn_err);


static Name *SearchNameStackTyped(Stack *stack, const char *name_buf, NameType type)
{
    __attribute__((cleanup(StackDtor)))
    Stack stack_temp = StackCtor();

    NamesTable *table_temp = NULL;
    Name *name             = NULL;

    for(size_t i = stack->size; i > 0; i--)
    {
        table_temp = (NamesTable *)PopStack(stack);
        PushStack(&stack_temp, table_temp);

        name = SearchNameTyped(table_temp, name_buf, type);
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


#define SPECIAL_CH(enum, ch) case ch: return true;
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


static int ScanWord(const char *str, size_t *pos, char buffer[])
{
    size_t buf_pos = 0;

    while(!(isspace(str[*pos]) || (IsSpecialCharacter(str[*pos])) || (str[*pos] == '\0')))
    {
        if(buf_pos == (BUFSIZ / 16))
        {
            return EXIT_FAILURE;
        }

        buffer[buf_pos++] = str[(*pos)++];
    }

    return EXIT_SUCCESS;
}

static double ScanDouble(const char *str, size_t *pos)
{
    char *end_p   = NULL;
    double result = strtod(str + *pos, &end_p);

    *pos = (size_t)(end_p - str);

    return result;
}


#define LINE code->lines[line - 1]
#define CHAR code->lines[line - 1][pos]
#define SPECIAL_CH(enum, ch) case ch: {AddToken(&tokens, {.sp_ch = SpecialChar::enum}, NodeType::SP_CH, line, pos++); break;}
static Tokens Tokenizator(Text *code)
{
    Tokens tokens = TokensCtor();

    size_t pos  = 0;
    size_t line = 1;

    for(; line <= code->n_lines; line++)
    {
        pos = 0;

        while(CHAR != '\0')
        {
            bool is_end = SkipSpaces(LINE, &pos);
            if(is_end) break;

            size_t old_pos = pos;

            if(isdigit(CHAR))
            {
                double dbl  = ScanDouble(LINE, &pos);
                AddToken(&tokens, {.num = dbl}, NodeType::NUM, line, old_pos);
            }
            else
            {
                switch(CHAR)
                {
                    #include "../../general/language/special_ch.h"
                    default:
                    {
                        char name_buf[BUFSIZ / 16] = {};

                        int exit_status = ScanWord(LINE, &pos, name_buf);
                        if(exit_status == EXIT_FAILURE)
                        {
                            printf("Name is too long.\n");
                            SyntaxErrMessage(code, line, old_pos);

                            TokensDtor(&tokens);
                            return {};
                        }

                        Name *name = SearchName(LANG, name_buf);

                        if(name)
                        {
                            AddToken(&tokens, {.name = name}, NodeType::NAME, line, old_pos);
                        }
                        else
                        {
                            AddToken(&tokens, {.word = strdup(name_buf)}, NodeType::WORD, line, old_pos);
                        }
                    }
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
#undef SPECIAL_CH


Tree ParseCode(const char *file_name)
{
    __attribute__((cleanup(BufDtor)))
    Buffer buf = {};

    __attribute__((cleanup(TextDtor)))
    Text code  = {};

    code = ReadText(file_name, &buf);
    if(!code.lines) return {};

    Tokens tokens = Tokenizator(&code);
    if(!tokens.data) return {};

    Token *current  = FirstToken(&tokens);
    bool is_syn_err = false;

    Node *root = ParseGeneral(&current, &is_syn_err);
    if(is_syn_err || (!root) || (current != tokens.data))
    {
        SyntaxErrMessage(&code, current->line, current->pos);

        TokensDtor(&tokens);
        return {};
    }

    TokensClear(&tokens);

    Tree tree = {.root = root};
    return tree;
}

#define LEXEME (*token)->lexeme
#define TOKEN_NEXT (*token) = (*token)->next
#define TOKEN_PREV (*token) = (*token)->prev

static Node *ParseGeneral(Token **token, bool *is_syn_err)
{
    __attribute__((cleanup(StackDtor)))
    Stack nt_stack = StackCtor();

    NamesTable *global = NamesTableCtor();
    PushStack(&nt_stack, global);

    TABLE_OF_TABLES[TABLE_POS++] = global;

    Node *ans   = NULL;
    Node **next = &ans;

    while(true)
    {
                   *next = ParseAssignment(&nt_stack, token, is_syn_err);
        if(!*next) *next = ParseFunction  (&nt_stack, token, is_syn_err);
        if(!*next) break;

        next = &(*next)->right;
    }
    if(*is_syn_err) return NULL;

    return ans;
}


static Node *ParseFunction(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    NamesTable *global = (NamesTable *)GetStackTop(nt_stack);

    Node *kword = LEXEME;
    SYN_ASSERT(IsKeyword(kword) && (GetKeyword(kword) == Keyword::FUNC), false);

    TOKEN_NEXT;

    Node *func = LEXEME;
    SYN_ASSERT(IsWord(func), true);

    TOKEN_NEXT;

    char *func_name = GetWord(func);
    SYN_ASSERT(SearchNameTyped(global, func_name, NameType::FUNC) == NULL, true);

    WordToName(global, func, NameType::FUNC, {.n_args = 0});

    NamesTable *local = NamesTableCtor();
    PushStack(nt_stack, local);

    TABLE_OF_TABLES[TABLE_POS++] = local;

    Node *args = ParseFuncArgs(local, token, is_syn_err, &func->data.name->val.n_args);
    if(*is_syn_err) return NULL;

    Node *body = ParseBody(nt_stack, token, is_syn_err);
    SYN_ASSERT(body, true);

    kword->left = func;
    func->left  = body;
    func->right = args;

    PopStack(nt_stack);

    return kword;
}

static Node *ParseFuncArgs(NamesTable *local, Token **token, bool *is_syn_err, size_t *n_args)
{
    Node *args  = NULL;
    Node **next = &args;

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT), true);

    do {
        TOKEN_NEXT;

        *next = LEXEME;
        SYN_ASSERT(IsWord(*next), *n_args != 0);

        (*n_args)++;
        WordToName(local, *next, NameType::VAR, {.var_val = 0});

        next = &(*next)->right;
        TOKEN_NEXT;
    }
    while(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::COMMA));

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT), true);

    TOKEN_NEXT;

    return args;
}


static Node *ParseIf(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *_if = LEXEME;
    SYN_ASSERT(IsKeyword(_if) && (GetKeyword(_if) == Keyword::IF), false);

    InsertToken(*token, {.sp_ch = SpecialChar::SEMICOLON}, NodeType::SP_CH, (*token)->line, (*token)->pos);
    InsertToken(*token, {.sp_ch = SpecialChar::SEMICOLON}, NodeType::SP_CH, (*token)->line, (*token)->pos);

    TOKEN_NEXT;
    _if->left = LEXEME;

    TOKEN_NEXT;
    _if->left->left = LEXEME;

    TOKEN_NEXT;

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT), true);

    TOKEN_NEXT;

    Node *_if_cond = ParseExpression(nt_stack, token, is_syn_err);
    SYN_ASSERT(_if_cond, true);

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT), true);

    TOKEN_NEXT;

    Node *_if_body = ParseBody(nt_stack, token, is_syn_err);
    SYN_ASSERT(_if_body, true);

    Node *_else = LEXEME;
    if(IsKeyword(_else) && (GetKeyword(_else) == Keyword::ELSE))
    {
        TOKEN_NEXT;

        _else->left = ParseBody(nt_stack, token, is_syn_err);
        SYN_ASSERT(_else->left, true);
    }
    else
        _else = NULL;

    _if->left->right = _else;
    _if->left->left->left  = _if_body;
    _if->left->left->right = _if_cond;

    return _if;
}

static Node *ParseWhile(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *_while = LEXEME;
    SYN_ASSERT(IsKeyword(_while) && (GetKeyword(_while) == Keyword::WHILE), false);

    InsertToken(*token, {.sp_ch = SpecialChar::SEMICOLON}, NodeType::SP_CH, (*token)->line, (*token)->pos);

    TOKEN_NEXT;
    _while->left = LEXEME;

    TOKEN_NEXT;

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT), true);

    TOKEN_NEXT;

    Node *_while_cond = ParseExpression(nt_stack, token, is_syn_err);
    SYN_ASSERT(_while_cond, true);

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT), true);

    TOKEN_NEXT;

    Node *_while_body = ParseBody(nt_stack, token, is_syn_err);
    SYN_ASSERT(_while_body, true);

    _while->left->left  = _while_body;
    _while->left->right = _while_cond;

    return _while;
}


static Node *ParseBody(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *ans   = NULL;
    Node **next = &ans;

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::CURLY_BRACKET_LEFT), true);

    TOKEN_NEXT;

    while(true)
    {
                   *next = ParseAssignment(nt_stack, token, is_syn_err);
        if(!*next) *next = ParseWhile     (nt_stack, token, is_syn_err);
        if(!*next) *next = ParseIf        (nt_stack, token, is_syn_err);
        if(!*next) *next = ParseReturn    (nt_stack, token, is_syn_err);
        if(!*next) break;

        next = &(*next)->right;
    }
    if((*is_syn_err)) return NULL;

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::CURLY_BRACKET_RIGHT), true);

    TOKEN_NEXT;

    return ans;
}

static Node *ParseAssignment(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *variable = LEXEME;
    SYN_ASSERT(IsWord(variable), false);

    TOKEN_NEXT;

    Node *ass = LEXEME;
    SYN_ASSERT(IsOperator(ass) && (GetOperator(ass) == Operator::ASS), true);

    TOKEN_NEXT;

    Node *expr = ParseExpression(nt_stack, token, is_syn_err);
    SYN_ASSERT(expr, true);

    Node *sep = LEXEME;
    SYN_ASSERT(IsSpecialChar(sep) && (GetSpecialChar(sep) == SpecialChar::SEMICOLON), true);

    TOKEN_NEXT;

    char *var_name = GetWord(variable);
    Name *name     = SearchNameStackTyped(nt_stack, var_name, NameType::VAR);

    if(!name)
    {
        NamesTable *names_space = (NamesTable *)GetStackTop(nt_stack);
        WordToName(names_space, variable, NameType::VAR, {.var_val = 0});
    }
    else
    {
        variable->type = NodeType::NAME;
        variable->data = {.name = name};
        free(var_name);
    }

    sep->left  = ass;
    ass->left  = variable;
    ass->right = expr;

    return sep;
}

static Node *ParseReturn(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *ret = LEXEME;
    SYN_ASSERT(IsKeyword(ret) && (GetKeyword(ret) == Keyword::RET), false);

    TOKEN_NEXT;

    Node *expr = ParseExpression(nt_stack, token, is_syn_err);
    SYN_ASSERT(expr, true);

    Node *sep = LEXEME;
    SYN_ASSERT(IsSpecialChar(sep) && (GetSpecialChar(sep) == SpecialChar::SEMICOLON), true);

    TOKEN_NEXT;

    sep->left = ret;
    ret->left = expr;

    return sep;
}



static Node *ParseExpression(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *expr = ParseExprAddSub(nt_stack, token, is_syn_err);
    SYN_ASSERT(expr, false);

    Node *ret_val = expr;
    Node *op      = LEXEME;

    if(IsOperator(op) && ((GetOperator(op) == Operator::EQ   ) || (GetOperator(op) == Operator::NOTEQ  ) ||
                          (GetOperator(op) == Operator::LESS ) || (GetOperator(op) == Operator::LESSEQ ) ||
                          (GetOperator(op) == Operator::ABOVE) || (GetOperator(op) == Operator::ABOVEEQ)))
    {
        TOKEN_NEXT;

        expr = ParseExprAddSub(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr, true);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;
    }

    return ret_val;
}

static Node *ParseExprAddSub(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *expr = NULL;
    Node *op   = LEXEME;

    if(IsOperator(op) && ((GetOperator(op) == Operator::ADD) ||
                          (GetOperator(op) == Operator::SUB)))
    {
        InsertToken((*token), {.num = 0}, NodeType::NUM, (*token)->line, (*token)->pos);
        TOKEN_NEXT;

        expr = LEXEME;
    }
    else
    {
        expr = ParseExprMulDiv(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr, false);

        op = LEXEME;
    }

    Node *ret_val = expr;

    while(IsOperator(op) && ((GetOperator(op) == Operator::ADD) ||
                             (GetOperator(op) == Operator::SUB)))
    {
        TOKEN_NEXT;

        expr = ParseExprMulDiv(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr, true);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprMulDiv(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *expr = ParseExprPow(nt_stack, token, is_syn_err);
    SYN_ASSERT(expr, false);

    Node *ret_val = expr;
    Node *op      = LEXEME;

    while(IsOperator(op) && ((GetOperator(op) == Operator::MUL) ||
                             (GetOperator(op) == Operator::DIV)))
    {
        TOKEN_NEXT;

        expr = ParseExprPow(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr, true);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprPow(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *expr = ParseExprPrimary(nt_stack, token, is_syn_err);
    SYN_ASSERT(expr, false);

    Node *ret_val = expr;
    Node *op      = LEXEME;

    while(IsOperator(op) && (GetOperator(op) == Operator::POW))
    {
        TOKEN_NEXT;

        expr = ParseExprPrimary(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr, true);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprPrimary(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *ret_val = LEXEME;

    switch(ret_val->type)
    {
        case NodeType::NUM:
        {
            TOKEN_NEXT;
            break;
        }
        case NodeType::WORD:
        {
            TOKEN_NEXT;

            char *var_name = GetWord(ret_val);
            Name *name = SearchNameStackTyped(nt_stack, var_name, NameType::VAR);
            SYN_ASSERT(name, false);

            ret_val->type = NodeType::NAME;
            ret_val->data = {.name = name};
            free(var_name);

            break;
        }
        case NodeType::SP_CH:
        {
            ret_val = ParseExprBrackets(nt_stack, token, is_syn_err);

            break;
        }
        case NodeType::NAME:
        default: return NULL;
    }

    return ret_val;
}

static Node *ParseExprBrackets(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT), true);

    TOKEN_NEXT;

    Node *ret_val = ParseExpression(nt_stack, token, is_syn_err);
    if(!ret_val) return NULL;

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT), true);

    TOKEN_NEXT;

    return ret_val;
}

#undef LEXEME
#undef TOKEN_NEXT
#undef TOKEN_PREV