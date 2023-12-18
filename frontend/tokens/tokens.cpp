#include <stdlib.h>

#include "tokens.h"

Tokens TokensCtor(void)
{
    Tokens tokens = {};

    tokens.data  = (Token *)calloc(1, sizeof(Token));
    if(!tokens.data)
    {
        LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);

        return {};
    }
    *tokens.data = {.lexeme = NodeCtor({.sp_ch = SpecialChar::END}, NodeType::SP_CH)};

    tokens.data->prev = tokens.data;
    tokens.data->next = tokens.data;

    return tokens;
}


void TokensDtor(Tokens *tokens)
{
    Token *head = LastToken(tokens);

    for(Token *pos = FirstToken(tokens); pos != tokens->data; pos = pos->next)
    {
        NodeDtor(pos->prev->lexeme);
        free(pos->prev);
    }

    NodeDtor(head->lexeme);
    free(head);

    tokens->data = NULL;
}

void TokensClear(Tokens *tokens)
{
    Token *head = LastToken(tokens);

    for(Token *pos = FirstToken(tokens); pos != tokens->data; pos = pos->next)
    {
        if(IsSpecialChar(pos->prev->lexeme) && !(GetSpecialChar(pos->prev->lexeme) == SpecialChar::SEMICOLON) &&
                                               !(GetSpecialChar(pos->prev->lexeme) == SpecialChar::COMMA))
        {
            NodeDtor(pos->prev->lexeme);
        }
        free(pos->prev);
    }

    if(IsSpecialChar(head->lexeme) && !(GetSpecialChar(head->lexeme) == SpecialChar::SEMICOLON) &&
                                      !(GetSpecialChar(head->lexeme) == SpecialChar::COMMA))
    {
        NodeDtor(head->lexeme);
    }
    free(head);

    tokens->data = NULL;
}

Token *LastToken(Tokens *tokens)
{
    return tokens->data->prev;
}

Token *FirstToken(Tokens *tokens)
{
    return tokens->data->next;
}


Token *AddToken(Tokens *tokens, node_t data, NodeType type, size_t line, size_t l_pos)
{
    Token *head = LastToken(tokens);
    return InsertToken(head, data, type, line, l_pos);
}

Token *InsertToken(Token *prev_p, node_t data, NodeType type, size_t line, size_t l_pos)
{
    Token *token  = (Token *)calloc(1, sizeof(Token));
    *token = {.lexeme = NodeCtor(data, type),
              .line   = line,
              .pos    = l_pos};

    token->next        = prev_p->next;
    prev_p->next->prev = token;

    token->prev  = prev_p;
    prev_p->next = token;

    return token;
}

Token *TokenRemove(Token *del_p)
{
    del_p->prev->next = del_p->next;
    del_p->next->prev = del_p->prev;

    Token *ret = del_p->next;

    NodeDtor(del_p->lexeme);
    free(del_p);

    return ret;
}