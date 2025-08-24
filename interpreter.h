#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"

void init_interpreter(void);
void interpret(Token* token);
void deinit_interpreter(void);

#endif /* INTERPRETER_H */
