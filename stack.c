#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

numstack* numstack_init(void) {
    number* data = calloc(NUMSTACK_INIT_CAP, sizeof(number));
    if (!data) {
        fprintf(stderr, "Could not allocate data for numstack\n");
        abort();
    }

    numstack* stack = malloc(sizeof(numstack));
    if (!stack) {
        free(data);
        fprintf(stderr, "Could not allocate numstack header\n");
        abort();
    }

    stack->data = data;
    stack->offset = 0;
    stack->cap = NUMSTACK_INIT_CAP;
    return stack;
}

void __numstack_resize(numstack* stack) {
    number* new_data = realloc(stack->data, stack->cap*2);
    if (!new_data) {
        numstack_deinit(stack);
        fprintf(stderr, "Could not expand numstack\n");
        abort();
    }

    stack->data = new_data;
    stack->cap *= 2;
}

void numstack_deinit(numstack* stack) {
    free(stack->data);
    free(stack);
}

void numstack_push(numstack* stack, number num) {
    if (stack->offset == stack->cap)
        __numstack_resize(stack);

    stack->data[stack->offset] = num;
    stack->offset++;
}

number numstack_pop(numstack* stack) {
    if (stack->offset <= 0) {
        fprintf(stderr, "Attempt to pop from empty numstack\n");
        abort();
    }

    stack->offset--;
    return stack->data[stack->offset];
}
