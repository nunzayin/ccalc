#ifndef STACK_H
#define STACK_H

#include "number.h"

const size_t NUMSTACK_INIT_CAP = 32;

typedef struct {
    number* data;
    size_t offset;
    size_t cap;
} numstack;

numstack* numstack_init(void);

void numstack_deinit(numstack* stack);

void numstack_push(numstack* stack, number num);

number numstack_pop(numstack* stack);

#endif /* STACK_H */
