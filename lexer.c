#include "lexer.h"
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_SINGLE_CHECK_LIST \
    X(dot, '.') \
    X(e, 'e') \
    X(E, 'E') \
    X(plus, '+') \
    X(minus, '-') \
    X(star, '*') \
    X(slash, '/') \
    X(eqsign, '=')
#define X(cname, cval) \
    bool __char_is_##cname(unsigned char c) { \
        return c == cval;\
    }
CHAR_SINGLE_CHECK_LIST
#undef X
#define __fptr_char_is(cname) __char_is_##cname

bool __char_is_e_or_E(unsigned char c) {
    return c == 'e' || c == 'E';
}

#define CHAR_KINDS_LIST \
    X(CHAR_NUMERIC, isdigit) \
    X(CHAR_DOT, __fptr_char_is(dot)) \
    X(CHAR_E, __char_is_e_or_E) \
    X(CHAR_PLUS, __fptr_char_is(plus)) \
    X(CHAR_MINUS, __fptr_char_is(minus)) \
    X(CHAR_STAR, __fptr_char_is(star)) \
    X(CHAR_SLASH, __fptr_char_is(slash)) \
    X(CHAR_EQSIGN, __fptr_char_is(eqsign)) \
    X(CHAR_WS, isspace)

typedef enum {
#define X(ce, cf) \
    ce,
    CHAR_KINDS_LIST
#undef X
    CHAR_OTHER
} CharKind;

CharKind detect_char_kind(unsigned char c) {
#define X(ce, cf) \
    if (cf(c)) return ce;
    CHAR_KINDS_LIST
#undef X
    return CHAR_OTHER;
}

const size_t MIN_STRING_CAPACITY = 32;

Token* init_token(void) {
    size_t cap = MIN_STRING_CAPACITY;
    char* data = calloc(cap, sizeof(char));
    if (!data) {
        fprintf(stderr, "Could not allocate string for token\n");
        abort();
    };

    Token* token = malloc(sizeof(Token));
    if (!token) {
        fprintf(stderr, "Could not allocate token struct\n");
        abort();
    };

    token->kind = TOKEN_SKIP;
    token->data = data;
    token->cap = cap;
    return token;
}

void deinit_token(Token* token) {
    free(token->data);
    free(token);
}

void append_char(Token* token, char c) {
    size_t chars_len = strlen(token->data); // without \0

    if (chars_len+2 > token->cap) {
        char* new_data = realloc(token->data, token->cap*2*sizeof(char));
        if (!new_data) {
            fprintf(stderr, "Could not reallocate string for token\n");
            abort();
        }

        token->data = new_data;
        token->cap *= 2;
    }

    token->data[chars_len] = c;
    token->data[chars_len+1] = '\0';
}

const size_t MIN_TOKEN_LIST_CAPACITY = 16;

TokenList* init_token_list(void) {
    size_t cap = MIN_TOKEN_LIST_CAPACITY;
    Token** head = calloc(cap, sizeof(Token*));
    if (!head) {
        fprintf(stderr, "Could not allocate data for token list\n");
        abort();
    }

    TokenList* token_list = malloc(sizeof(TokenList));
    if (!token_list) {
        fprintf(stderr, "Could not allocate token list struct\n");
        abort();
    }

    token_list->head = head;
    token_list->len = 0;
    token_list->cap = cap;

    return token_list;
}

void deinit_token_list(TokenList* token_list) {
    for (size_t i = 0; i < token_list->len; i++)
        deinit_token(token_list->head[i]);
    free(token_list);
}

void append_token(TokenList* dest, Token* token) {
    if (dest->len == dest->cap) {
        Token** new_token_head = realloc(dest->head, dest->cap*2*sizeof(Token*));
        if (!new_token_head) {
            fprintf(stderr, "Could not reallocate token array for token list struct\n");
            abort();
        }

        dest->head = new_token_head;
        dest->cap *= 2;
    }

    dest->len++;
    dest->head[dest->len-1] = token;
}

#define TOKENIZER_STATE_LIST \
    X(TOKENIZER_STATE_INIT, TOKEN_SKIP) \
    X(TOKENIZER_STATE_NUM_INT, TOKEN_NUMBER) \
    X(TOKENIZER_STATE_NUM_DOT, TOKEN_NUMBER) \
    X(TOKENIZER_STATE_ADD, TOKEN_ADDITION) \
    X(TOKENIZER_STATE_SUB, TOKEN_SUBTRACTION) \
    X(TOKENIZER_STATE_MULT, TOKEN_MULTIPLICATION) \
    X(TOKENIZER_STATE_DIV, TOKEN_DIVISION) \
    X(TOKENIZER_STATE_PRT, TOKEN_PRINT) \
    X(TOKENIZER_STATE_WS, TOKEN_SKIP) \
    X(TOKENIZER_STATE_COMM, TOKEN_SKIP) \
    X(TOKENIZER_STATE_NUM_E, TOKEN_NUMBER) \
    X(TOKENIZER_STATE_NUM_FRAC, TOKEN_NUMBER) \
    X(TOKENIZER_STATE_NUM_EXP, TOKEN_NUMBER) \
    X(TOKENIZER_STATE_NUM_E_NP, TOKEN_NUMBER) \
    X(TOKENIZER_STATE_ERR, TOKEN_SKIP)

typedef enum {
#define X(ts, tk) \
    ts,
    TOKENIZER_STATE_LIST
#undef X
} TokenizerState;

const TokenKind TOKEN_KIND_BY_TOKENIZER_STATE[/*TokenizerState*/] = {
#define X(ts, tk) \
    [ts] = tk,
    TOKENIZER_STATE_LIST
#undef X
};

typedef struct {
    TokenizerState new_state;
    bool do_split;
} TokenDigestRule;

const TokenDigestRule TOKENIZER_RULESET[/*TokenizerState*/][/*CharKind*/ CHAR_OTHER+1] = {
    [TOKENIZER_STATE_INIT][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, true},
    [TOKENIZER_STATE_INIT][CHAR_DOT] = {TOKENIZER_STATE_NUM_DOT, true},
    [TOKENIZER_STATE_INIT][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_INIT][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_INIT][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_INIT][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_INIT][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_INIT][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_INIT][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_INIT][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_NUM_INT][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, false},
    [TOKENIZER_STATE_NUM_INT][CHAR_DOT] = {TOKENIZER_STATE_NUM_DOT, false},
    [TOKENIZER_STATE_NUM_INT][CHAR_E] = {TOKENIZER_STATE_NUM_E, false},
    [TOKENIZER_STATE_NUM_INT][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_NUM_INT][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_NUM_INT][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_NUM_INT][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_NUM_INT][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_NUM_INT][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_NUM_INT][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_NUM_DOT][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_FRAC, false},
    [TOKENIZER_STATE_NUM_DOT][CHAR_DOT] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_PLUS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_MINUS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_STAR] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_SLASH] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_EQSIGN] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_WS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_DOT][CHAR_OTHER] = {TOKENIZER_STATE_ERR, true},

    [TOKENIZER_STATE_ADD][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, true},
    [TOKENIZER_STATE_ADD][CHAR_DOT] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_ADD][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_ADD][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_ADD][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_ADD][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_ADD][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_ADD][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_ADD][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_ADD][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_SUB][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, false},
    [TOKENIZER_STATE_SUB][CHAR_DOT] = {TOKENIZER_STATE_NUM_DOT, false},
    [TOKENIZER_STATE_SUB][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_SUB][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_SUB][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_SUB][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_SUB][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_SUB][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_SUB][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_SUB][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_MULT][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, true},
    [TOKENIZER_STATE_MULT][CHAR_DOT] = {TOKENIZER_STATE_NUM_DOT, true},
    [TOKENIZER_STATE_MULT][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_MULT][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_MULT][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_MULT][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_MULT][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_MULT][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_MULT][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_MULT][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_DIV][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, true},
    [TOKENIZER_STATE_DIV][CHAR_DOT] = {TOKENIZER_STATE_NUM_DOT, true},
    [TOKENIZER_STATE_DIV][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_DIV][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_DIV][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_DIV][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_DIV][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_DIV][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_DIV][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_DIV][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_PRT][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, true},
    [TOKENIZER_STATE_PRT][CHAR_DOT] = {TOKENIZER_STATE_NUM_DOT, true},
    [TOKENIZER_STATE_PRT][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_PRT][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_PRT][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_PRT][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_PRT][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_PRT][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_PRT][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_PRT][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_WS][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_INT, true},
    [TOKENIZER_STATE_WS][CHAR_DOT] = {TOKENIZER_STATE_NUM_DOT, true},
    [TOKENIZER_STATE_WS][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_WS][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_WS][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_WS][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_WS][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_WS][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_WS][CHAR_WS] = {TOKENIZER_STATE_WS, false},
    [TOKENIZER_STATE_WS][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_COMM][CHAR_NUMERIC] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_DOT] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_E] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_PLUS] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_MINUS] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_STAR] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_SLASH] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_EQSIGN] = {TOKENIZER_STATE_COMM, false},
    [TOKENIZER_STATE_COMM][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_COMM][CHAR_OTHER] = {TOKENIZER_STATE_COMM, false},

    [TOKENIZER_STATE_NUM_E][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_EXP, false},
    [TOKENIZER_STATE_NUM_E][CHAR_DOT] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E][CHAR_PLUS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E][CHAR_MINUS] = {TOKENIZER_STATE_NUM_E_NP, false},
    [TOKENIZER_STATE_NUM_E][CHAR_STAR] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E][CHAR_SLASH] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E][CHAR_EQSIGN] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E][CHAR_WS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E][CHAR_OTHER] = {TOKENIZER_STATE_ERR, true},

    [TOKENIZER_STATE_NUM_FRAC][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_FRAC, false},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_DOT] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_E] = {TOKENIZER_STATE_NUM_E, false},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_NUM_FRAC][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_NUM_EXP][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_EXP, false},
    [TOKENIZER_STATE_NUM_EXP][CHAR_DOT] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_PLUS] = {TOKENIZER_STATE_ADD, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_MINUS] = {TOKENIZER_STATE_SUB, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_STAR] = {TOKENIZER_STATE_MULT, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_SLASH] = {TOKENIZER_STATE_DIV, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_EQSIGN] = {TOKENIZER_STATE_PRT, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_WS] = {TOKENIZER_STATE_WS, true},
    [TOKENIZER_STATE_NUM_EXP][CHAR_OTHER] = {TOKENIZER_STATE_COMM, true},

    [TOKENIZER_STATE_NUM_E_NP][CHAR_NUMERIC] = {TOKENIZER_STATE_NUM_EXP, false},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_DOT] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_E] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_PLUS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_MINUS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_STAR] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_SLASH] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_EQSIGN] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_WS] = {TOKENIZER_STATE_ERR, true},
    [TOKENIZER_STATE_NUM_E_NP][CHAR_OTHER] = {TOKENIZER_STATE_ERR, true},
};

typedef struct {
    TokenizerState current_state;
    Token* token;
    TokenList* output_list;
} Tokenizer;

Tokenizer* init_tokenizer(TokenList* token_list) {
    Tokenizer* tokenizer = malloc(sizeof(Tokenizer));
    if (!tokenizer) {
        fprintf(stderr, "Could not allocate tokenizer struct\n");
        abort();
    }

    tokenizer->current_state = TOKENIZER_STATE_INIT;
    tokenizer->token = init_token();
    tokenizer->output_list = token_list;
    return tokenizer;
}

void update_token_kind(Tokenizer* tokenizer) {
    tokenizer->token->kind = TOKEN_KIND_BY_TOKENIZER_STATE[tokenizer->current_state];
}

void push_token(Tokenizer* tokenizer, bool alloc_new) {
    append_token(tokenizer->output_list, tokenizer->token);
    tokenizer->token = alloc_new ? init_token() : NULL;
}

void digest_char(Tokenizer* tokenizer, unsigned char c) {
    TokenDigestRule rule = TOKENIZER_RULESET[tokenizer->current_state][detect_char_kind(c)];
    if (rule.do_split && tokenizer->token->kind != TOKEN_SKIP)
        push_token(tokenizer, true);
    tokenizer->current_state = rule.new_state;
    update_token_kind(tokenizer);
    if (tokenizer->token->kind == TOKEN_SKIP) return;
    append_char(tokenizer->token, c);
}

void deinit_tokenizer(Tokenizer* tokenizer) {
    if (tokenizer->token->kind != TOKEN_SKIP)
        push_token(tokenizer, false);
    free(tokenizer);
}

TokenList* tokenize(FILE* file) {
    TokenList* token_list = init_token_list();
    Tokenizer* tokenizer = init_tokenizer(token_list);

    int c;
    while ((c = getc(file)) != EOF) {
        digest_char(tokenizer, c);
    }

    deinit_tokenizer(tokenizer);
    return token_list;
}
