#ifndef TOKENS_H
#define TOKENS_H

#include <stddef.h>

#include "../../general/log/log.h"
#include "../../general/tree/include/tree.h"

struct Token
{
    Node *lexeme;

    size_t line;
    size_t pos;

    Token *next;
    Token *prev;
};

struct Tokens
{
    Token *data;
};

Tokens TokensCtor(void);

void TokensDtor(Tokens *tokens);

void TokensClear(Tokens *tokens);

Token *LastToken(Tokens *tokens);

Token *FirstToken(Tokens *tokens);

Token *AddToken(Tokens *tokens, node_t data, NodeType type, size_t line, size_t l_pos);

Token *InsertToken(Token *prev_p, node_t data, NodeType type, size_t line, size_t l_pos);

Token *TokenRemove(Token *del_p);

#endif //TOKENS_H