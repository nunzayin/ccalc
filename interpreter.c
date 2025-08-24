#include "interpreter.h"
#include "stack.h"
#include <stdlib.h>

numstack* stack = NULL;

void push_number(char* data) {
    numstack_push(stack, atof(data));
}

void add(char* data) {
    numstack_push(stack, numstack_pop(stack) + numstack_pop(stack));
}

void subtract(char* data) {
    number num = numstack_pop(stack);
    numstack_push(stack, numstack_pop(stack) - num);
}

void multiply(char* data) {
    numstack_push(stack, numstack_pop(stack) * numstack_pop(stack));
}

void divide(char* data) {
    number num = numstack_pop(stack);
    if (num == 0) {
        fprintf(stderr, "Attempt to divide by zero\n");
        abort();
    }
    numstack_push(stack, numstack_pop(stack) / num);
}

void print_number(char* data) {
    printf("%g\n", numstack_pop(stack));
}

void (*TOKEN_INTERPRET_VTABLE[/*TokenKind*/])(char*) = {
    [TOKEN_NUMBER] = push_number,
    [TOKEN_ADDITION] = add,
    [TOKEN_SUBTRACTION] = subtract,
    [TOKEN_MULTIPLICATION] = multiply,
    [TOKEN_DIVISION] = divide,
    [TOKEN_PRINT] = print_number,
};

void init_interpreter(void) {
    stack = numstack_init();
}

void interpret(Token* token) {
    if (!stack) {
        fprintf(stderr, "Attempt to work on uninitialized stack\n");
        abort();
    }
    TOKEN_INTERPRET_VTABLE[token->kind](token->data);
}

void deinit_interpreter(void) {
    while (stack->offset > 0)
        fprintf(stderr, "Unused value on stack: %g\n",
                numstack_pop(stack));
    numstack_deinit(stack);
}
