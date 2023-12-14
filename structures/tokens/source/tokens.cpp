#include <stdlib.h>

#include "../include/tokens.h"

Tokens TokensCtor(void)
{
    Tokens tokens = {};

    tokens.data  = (Token *)calloc(1, sizeof(Token));
    *tokens.data = {.lexeme = NodeCtor({.tmp = '\0'}, NodeType::TMP)};

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
        if(pos->prev->lexeme->type == NodeType::TMP)
        {
            NodeDtor(pos->prev->lexeme);
        }
        free(pos->prev);
    }

    if(head->lexeme->type == NodeType::TMP)
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