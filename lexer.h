#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdio.h>

typedef enum {
    TOKEN_SKIP = -1,
    TOKEN_NUMBER,
    TOKEN_ADDITION,
    TOKEN_SUBTRACTION,
    TOKEN_MULTIPLICATION,
    TOKEN_DIVISION,
    TOKEN_PRINT
} TokenKind;

typedef struct {
    TokenKind kind;
    char* data;
    size_t cap;
} Token;

typedef struct {
    Token** head;
    size_t len;
    size_t cap;
} TokenList;

TokenList* tokenize(FILE* file);
void deinit_token_list(TokenList* token_list);

#endif /* LEXER_H */
