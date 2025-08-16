#ifndef TOKENBUF_H
#define TOKENBUF_H

const size_t TOKENBUF_CAP = 1024;

typedef enum {
    NUMERIC,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    PRINT,
    ANYELSE
} token_kind;

typedef struct tokenbuf_t {
    char data[TOKENBUF_CAP];
    token_kind kind;
    size_t len;
} tokenbuf;

tokenbuf tokenbuf_init(void);

#endif /* TOKENBUF_H */
