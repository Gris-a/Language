#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"

static NamesTable *LANG = NamesTableCtorLang();

static size_t TABLE_SIZE  = 1;
static size_t TABLE_POS   = 0;
static size_t VARIABLE_ID = 0;

static NamesTable **TABLE_OF_TABLES = (NamesTable **)calloc(TABLE_SIZE, sizeof(NamesTable *));

void ClearTables(void)
{
    NamesTableDtor(LANG);

    for(size_t i = 0; i < TABLE_POS; i++)
    {
        NamesTableDtor(TABLE_OF_TABLES[i]);
    }
    free(TABLE_OF_TABLES);
}

static int AddTable(NamesTable *table)
{
    if(TABLE_POS == TABLE_SIZE)
    {
        NamesTable **temp = (NamesTable **)realloc(TABLE_OF_TABLES, sizeof(NamesTable*) * (TABLE_SIZE *= 2));
        if(!temp)
        {
            LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);

            return EXIT_FAILURE;
        }

        TABLE_OF_TABLES = temp;
    }
    TABLE_OF_TABLES[TABLE_POS++] = table;

    return EXIT_SUCCESS;
}


static Name *SearchNameStack(Stack *stack, const char *name_buf);

static void SyntaxErrMessage(Text *code, size_t line, size_t l_pos);

static bool IsSpecialCharacter(char ch);

static bool SkipSpaces            (const char *str, size_t *pos);
static bool SkipUnwantedCharacters(const char *str, size_t *pos);

static double ScanDouble(const char *str, size_t *pos);
static int    ScanWord  (const char *str, size_t *pos, char buffer[]);

static Tokens Tokenizator(Text *code);

static Node *ParseGeneral(Token **token, bool *is_syn_err);

static Node *ParseFunction(Stack *nt_stack  , Token **token, bool *is_syn_err);
static Node *ParseFuncArgSingle(NamesTable * local, Token **token, bool *is_syn_err, size_t *n_args);
static Node *ParseFuncArgs      (NamesTable *local, Token **token, bool *is_syn_err, size_t *n_args);

static Node *ParseElse (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseIf   (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseWhile(Stack *nt_stack, Token **token, bool *is_syn_err);

static Node *ParseBody      (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseAssignment(Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseReturn    (Stack *nt_stack, Token **token, bool *is_syn_err);

static Node *ParseExprOrSep   (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprOr      (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprAnd     (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprComp    (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprAddSub  (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprMulDiv  (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprPow     (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprPrimary (Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprBrackets(Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprVariable(Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprFuncCall(Stack *nt_stack, Token **token, bool *is_syn_err);
static Node *ParseExprFuncCallArgs(Stack *nt_stack, Name *func, Token **token, bool *is_syn_err, size_t *n_args);


static Name *SearchNameStack(Stack *stack, const char *name_buf)
{
    __attribute__((cleanup(StackDtor)))
    Stack stack_temp = StackCtor();

    NamesTable *table_temp = NULL;
    Name *name             = NULL;

    while(stack->size > 0)
    {
        table_temp = (NamesTable *)PopStack(stack);
        PushStack(&stack_temp, table_temp);

        name = SearchNameTyped(table_temp, name_buf, NameType::VAR);
        if(name) break;
    }

    while(stack_temp.size > 0)
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

static bool SkipUnwantedCharacters(const char *str, size_t *pos)
{
    static bool is_comment = false;
    bool is_end = false;

    while(!(is_end = (str[*pos] == '\0')))
    {
        is_end = SkipSpaces(str, pos);
        if(is_end) break;

        if(is_comment || (str[*pos] == '@'))
        {
            if(!is_comment)
            {
                (*pos)++;
                is_comment = true;
            }

            while(str[*pos] != '\0')
            {
                if(str[(*pos)++] == '@')
                {
                    is_comment = false;
                    break;
                }
            }
        }
        else break;
    }

    return is_end;
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
            bool is_end = SkipUnwantedCharacters(LINE, &pos);
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
                            if(name->type == NameType::KWORD)
                                AddToken(&tokens, {.kword = name->val.kword}, NodeType::KWORD, line, old_pos);
                            else if(name->type == NameType::OP)
                                AddToken(&tokens, {.op = name->val.op}, NodeType::OP, line, old_pos);
                        }
                        else
                        {
                            Token *token = AddToken(&tokens, {.word = strdup(name_buf)}, NodeType::WORD, line, old_pos);
                            if(!GetWord(token->lexeme))
                            {
                                LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);
                            }
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
    atexit(ClearTables);

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
#define LEXEME_NEXT (*token)->next->lexeme
#define TOKEN_NEXT (*token) = (*token)->next
#define TOKEN_PREV (*token) = (*token)->prev

#define BUILTIN_FUNC(func_name, num_args) AddName(global, func_name, NameType::FUNC, {.n_args = num_args});
static Node *ParseGeneral(Token **token, bool *is_syn_err)
{
    __attribute__((cleanup(StackDtor)))
    Stack nt_stack = StackCtor();

    NamesTable *global = NamesTableCtor();
    AddTable(global);
    PushStack(&nt_stack, global);

    #include "../../general/language/functions.h"

    Node *ans   = NULL;
    Node **next = &ans;

    while(true)
    {
                   *next = ParseFunction  (&nt_stack, token, is_syn_err);
        if(!*next) *next = ParseAssignment(&nt_stack, token, is_syn_err);
        if(!*next) break;

        next = &(*next)->right;
    }
    SYN_ASSERT(!(*is_syn_err));

    SYN_ASSERT(SearchNameTyped(global, "main", NameType::FUNC));

    return ans;
}
#undef BUILTIN_FUNC


static Node *ParseFunction(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    NamesTable *global = (NamesTable *)GetStackTop(nt_stack);

    Node *ret_val = LEXEME;
    if(!(IsKeyword(ret_val) && (GetKeyword(ret_val) == Keyword::FUNC))) return NULL;
    TOKEN_NEXT;

    Node *func = LEXEME;
    SYN_ASSERT(IsWord(func));
    TOKEN_NEXT;

    char *func_name = GetWord(func);
    SYN_ASSERT(SearchNameTyped(global, func_name, NameType::FUNC) == NULL);

    Name *name = AddName(global, func_name, NameType::FUNC, {.n_args = 0});
    func->type = NodeType::FUNC;
    func->data = {.func = func_name};

    NamesTable *local = NamesTableCtor();
    AddTable(local);

    Node *args = ParseFuncArgs(local, token, is_syn_err, &name->val.n_args);
    SYN_ASSERT(!(*is_syn_err));

    PushStack(nt_stack, local);
    Node *body = ParseBody(nt_stack, token, is_syn_err);
    SYN_ASSERT(body);
    PopStack(nt_stack);

    ret_val->left = func;

    func->left  = body;
    func->right = args;


    return ret_val;
}

static Node *ParseFuncArgSingle(NamesTable * local, Token **token, bool *is_syn_err, size_t *n_args)
{
    Node *arg = LEXEME;
    SYN_ASSERT(IsWord(arg));
    TOKEN_NEXT;

    char *var_name = GetWord(arg);
    AddName(local, var_name, NameType::VAR, {.var_id = VARIABLE_ID});
    free(var_name);

    arg->type = NodeType::VAR;
    arg->data = {.var_id = (VARIABLE_ID++)};
    (*n_args)++;

    return arg;
}

static Node *ParseFuncArgs(NamesTable *local, Token **token, bool *is_syn_err, size_t *n_args)
{
    SYN_ASSERT(!*is_syn_err);

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT));

    if(IsSpecialChar(LEXEME_NEXT) && (GetSpecialChar(LEXEME_NEXT) == SpecialChar::BRACKET_RIGHT))
    {
        TOKEN_NEXT;
        TOKEN_NEXT;
        return NULL;
    }

    InsertToken(*token, {.sp_ch = SpecialChar::COMMA}, NodeType::SP_CH, (*token)->line, (*token)->pos);
    TOKEN_NEXT;

    Node *args  = NULL;
    Node **next = &args;

    while(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::COMMA))
    {
        *next = LEXEME;
        TOKEN_NEXT;

        Node *arg = ParseFuncArgSingle(local, token, is_syn_err, n_args);
        SYN_ASSERT(arg);

        (*next)->left    = arg;
        next = &(*next)->right;
    }

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT));
    TOKEN_NEXT;

    return args;
}


static Node *ParseElse(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    Node *_else = LEXEME;
    if(IsKeyword(_else) && (GetKeyword(_else) == Keyword::ELSE))
    {
        TOKEN_NEXT;

        NamesTable *_else_table = NamesTableCtor();
        AddTable(_else_table);

        PushStack(nt_stack, _else_table);
        _else->left = ParseBody(nt_stack, token, is_syn_err);
        SYN_ASSERT(_else->left);
        PopStack(nt_stack);

    }
    else _else = NULL;

    return _else;
}

static Node *ParseIf(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *_if = LEXEME;
    if(!(IsKeyword(_if) && (GetKeyword(_if) == Keyword::IF))) return NULL;

    InsertToken(*token, {.sp_ch = SpecialChar::SEMICOLON}, NodeType::SP_CH, (*token)->line, (*token)->pos);
    TOKEN_NEXT;
    Node *ret_val = LEXEME;

    InsertToken(*token, {.sp_ch = SpecialChar::SEMICOLON}, NodeType::SP_CH, (*token)->line, (*token)->pos);
    TOKEN_NEXT;
    _if->left = LEXEME;

    TOKEN_NEXT;
    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT));
    TOKEN_NEXT;

    Node *_if_cond = ParseExprOr(nt_stack, token, is_syn_err);
    SYN_ASSERT(_if_cond);

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT));
    TOKEN_NEXT;

    NamesTable *_if_table = NamesTableCtor();
    AddTable(_if_table);

    PushStack(nt_stack, _if_table);
    Node *_if_body = ParseBody(nt_stack, token, is_syn_err);
    SYN_ASSERT(_if_body);
    PopStack(nt_stack);

    Node *_else = ParseElse(nt_stack, token, is_syn_err);
    SYN_ASSERT(!(*is_syn_err));

    ret_val->left    = _if;
    _if->right       = _else;
    _if->left->left  = _if_body;
    _if->left->right = _if_cond;

    return ret_val;
}

static Node *ParseWhile(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *_while = LEXEME;
    if(!(IsKeyword(_while) && (GetKeyword(_while) == Keyword::WHILE))) return NULL;

    InsertToken(*token, {.sp_ch = SpecialChar::SEMICOLON}, NodeType::SP_CH, (*token)->line, (*token)->pos);
    TOKEN_NEXT;
    Node *ret_val = LEXEME;

    InsertToken(*token, {.sp_ch = SpecialChar::SEMICOLON}, NodeType::SP_CH, (*token)->line, (*token)->pos);
    TOKEN_NEXT;
    _while->left = LEXEME;

    TOKEN_NEXT;
    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT));
    TOKEN_NEXT;

    Node *_while_cond = ParseExprOr(nt_stack, token, is_syn_err);
    SYN_ASSERT(_while_cond);

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT));
    TOKEN_NEXT;

    NamesTable *_while_table = NamesTableCtor();
    AddTable(_while_table);

    PushStack(nt_stack, _while_table);
    Node *_while_body = ParseBody(nt_stack, token, is_syn_err);
    SYN_ASSERT(_while_body);
    PopStack(nt_stack);

    ret_val->left       = _while;
    _while->left->left  = _while_body;
    _while->left->right = _while_cond;

    return ret_val;
}


static Node *ParseBody(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *ans   = NULL;
    Node **next = &ans;

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::CURLY_BRACKET_LEFT));
    TOKEN_NEXT;

    while(true)
    {
                   *next = ParseWhile     (nt_stack, token, is_syn_err);
        if(!*next) *next = ParseIf        (nt_stack, token, is_syn_err);
        if(!*next) *next = ParseReturn    (nt_stack, token, is_syn_err);
        if(!*next) *next = ParseAssignment(nt_stack, token, is_syn_err);
        if(!*next) *next = ParseExprOrSep (nt_stack, token, is_syn_err);
        if(!*next) break;

        next = &(*next)->right;
    }
    SYN_ASSERT(!(*is_syn_err));

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::CURLY_BRACKET_RIGHT));
    TOKEN_NEXT;

    return ans;
}

static Node *ParseAssignment(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *variable = LEXEME;
    if(!IsWord(variable)) return NULL;

    Node *ass = LEXEME_NEXT;
    if(!(IsOperator(ass) && (GetOperator(ass) == Operator::ASS))) return NULL;
    TOKEN_NEXT;
    TOKEN_NEXT;

    Node *expr = ParseExprOr(nt_stack, token, is_syn_err);
    SYN_ASSERT(expr);

    Node *sep = LEXEME;
    SYN_ASSERT(IsSpecialChar(sep) && (GetSpecialChar(sep) == SpecialChar::SEMICOLON));
    TOKEN_NEXT;

    char *var_name = GetWord(variable);
    Name *name     = SearchNameStack(nt_stack, var_name);

    variable->type = NodeType::VAR;
    if(!name)
    {
        NamesTable *name_space = (NamesTable *)GetStackTop(nt_stack);
        AddName(name_space, var_name, NameType::VAR, {.var_id = VARIABLE_ID});
        variable->data = {.var_id = (VARIABLE_ID++)};
    }
    else variable->data = {.var_id = name->val.var_id};
    free(var_name);

    sep->left = ass;
    ass->left  = variable;
    ass->right = expr;

    return sep;
}

static Node *ParseReturn(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *ret = LEXEME;
    if(!(IsKeyword(ret) && (GetKeyword(ret) == Keyword::RET))) return NULL;
    TOKEN_NEXT;

    Node *expr = ParseExprOr(nt_stack, token, is_syn_err);
    SYN_ASSERT(expr);

    Node *sep = LEXEME;
    SYN_ASSERT(IsSpecialChar(sep) && (GetSpecialChar(sep) == SpecialChar::SEMICOLON));
    TOKEN_NEXT;

    sep->left = ret;
    ret->left = expr;

    return sep;
}


static Node *ParseExprOrSep(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *expr = ParseExprOr(nt_stack, token, is_syn_err);
    if(!expr) return NULL;

    Node *sep = LEXEME;
    SYN_ASSERT(IsSpecialChar(sep) && (GetSpecialChar(sep) == SpecialChar::SEMICOLON));
    TOKEN_NEXT;

    sep->left = expr;

    return sep;
}

static Node *ParseExprOr(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *expr = ParseExprAnd(nt_stack, token, is_syn_err);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op    = LEXEME;

    while(IsOperator(op) && (GetOperator(op) == Operator::OR))
    {
        TOKEN_NEXT;

        expr = ParseExprAnd(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprAnd(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *expr = ParseExprComp(nt_stack, token, is_syn_err);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op    = LEXEME;

    while(IsOperator(op) && (GetOperator(op) == Operator::AND))
    {
        TOKEN_NEXT;

        expr = ParseExprComp(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprComp(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *expr = ParseExprAddSub(nt_stack, token, is_syn_err);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op      = LEXEME;

    if(IsOperator(op) && ((GetOperator(op) == Operator::EQ   ) || (GetOperator(op) == Operator::NOTEQ  ) ||
                          (GetOperator(op) == Operator::LESS ) || (GetOperator(op) == Operator::LESSEQ ) ||
                          (GetOperator(op) == Operator::ABOVE) || (GetOperator(op) == Operator::ABOVEEQ)))
    {
        TOKEN_NEXT;

        expr = ParseExprAddSub(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;
    }

    return ret_val;
}

static Node *ParseExprAddSub(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    if(IsOperator(LEXEME) && ((GetOperator(LEXEME) == Operator::ADD) ||
                              (GetOperator(LEXEME) == Operator::SUB)))
    {
        TOKEN_PREV;
        InsertToken((*token), {.num = 0}, NodeType::NUM, (*token)->line, (*token)->pos);
        TOKEN_NEXT;
    }

    Node *expr = ParseExprMulDiv(nt_stack, token, is_syn_err);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op    = LEXEME;

    while(IsOperator(op) && ((GetOperator(op) == Operator::ADD) ||
                             (GetOperator(op) == Operator::SUB)))
    {
        TOKEN_NEXT;

        expr = ParseExprMulDiv(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprMulDiv(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *expr = ParseExprPow(nt_stack, token, is_syn_err);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op    = LEXEME;

    while(IsOperator(op) && ((GetOperator(op) == Operator::MUL) ||
                             (GetOperator(op) == Operator::DIV)))
    {
        TOKEN_NEXT;

        expr = ParseExprPow(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprPow(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *expr = ParseExprPrimary(nt_stack, token, is_syn_err);
    if(!expr) return NULL;

    Node *ret_val = expr;
    Node *op    = LEXEME;

    while(IsOperator(op) && (GetOperator(op) == Operator::POW))
    {
        TOKEN_NEXT;

        expr = ParseExprPrimary(nt_stack, token, is_syn_err);
        SYN_ASSERT(expr);

        op->left  = ret_val;
        op->right = expr;
        ret_val   = op;

        op = LEXEME;
    }

    return ret_val;
}

static Node *ParseExprPrimary(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

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
                         ret_val = ParseExprFuncCall(nt_stack, token, is_syn_err);
            if(!ret_val) ret_val = ParseExprVariable(nt_stack, token, is_syn_err);

            break;
        }
        case NodeType::SP_CH:
        {
            ret_val = ParseExprBrackets(nt_stack, token, is_syn_err);
            break;
        }
        case NodeType::FUNC:
        case NodeType::KWORD:
        case NodeType::OP:
        case NodeType::VAR:
        case NodeType::NOT_NODE:
        default: return NULL;
    }

    return ret_val;
}

static Node *ParseExprBrackets(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    if(!(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_LEFT))) return NULL;
    TOKEN_NEXT;

    Node *ret_val = ParseExprOr(nt_stack, token, is_syn_err);
    SYN_ASSERT(ret_val);

    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT));
    TOKEN_NEXT;

    return ret_val;
}

static Node *ParseExprFuncCallArgs(Stack *nt_stack, Name *func, Token **token, bool *is_syn_err, size_t *n_args)
{
    SYN_ASSERT(!*is_syn_err);

    if(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT))
    {
        SYN_ASSERT(func->val.n_args == 0);
        return NULL;
    }

    TOKEN_PREV;
    InsertToken(*token, {.sp_ch = SpecialChar::COMMA}, NodeType::SP_CH, (*token)->line, (*token)->pos);
    TOKEN_NEXT;

    Node *args  = NULL;
    Node **next = &args;

    while(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::COMMA))
    {
        SYN_ASSERT(*n_args < func->val.n_args);
        (*n_args)++;

        *next = LEXEME;
        TOKEN_NEXT;

        Node *arg = ParseExprOr(nt_stack, token, is_syn_err);
        SYN_ASSERT(arg);

        (*next)->left    = arg;
        next = &(*next)->right;
    }

    return args;
}

static Node *ParseExprFuncCall(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    if(!(IsSpecialChar(LEXEME_NEXT) && (GetSpecialChar(LEXEME_NEXT) == SpecialChar::BRACKET_LEFT))) return NULL;

    Node *func = LEXEME;
    TOKEN_NEXT;
    TOKEN_NEXT;

    char *func_name = GetWord(func);
    Name *name = SearchNameTyped(TABLE_OF_TABLES[0], func_name, NameType::FUNC);
    SYN_ASSERT(name);

    func->type = NodeType::FUNC;
    func->data = {.func = func_name};

    size_t n_args = 0;
    Node *args = ParseExprFuncCallArgs(nt_stack, name, token, is_syn_err, &n_args);
    SYN_ASSERT(!(*is_syn_err));

    SYN_ASSERT(n_args == name->val.n_args);
    SYN_ASSERT(IsSpecialChar(LEXEME) && (GetSpecialChar(LEXEME) == SpecialChar::BRACKET_RIGHT));
    TOKEN_NEXT;

    func->left = args;
    return func;
}

static Node *ParseExprVariable(Stack *nt_stack, Token **token, bool *is_syn_err)
{
    SYN_ASSERT(!*is_syn_err);

    Node *variable = LEXEME;

    char *var_name = GetWord(variable);
    Name *name = SearchNameStack(nt_stack, var_name);
    if(!name) return NULL;
    free(var_name);

    variable->type = NodeType::VAR;
    variable->data = {.var_id = name->val.var_id};
    TOKEN_NEXT;

    return variable;
}

#undef LEXEME
#undef LEXEME_NEXT
#undef TOKEN_NEXT
#undef TOKEN_PREV