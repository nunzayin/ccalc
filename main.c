#include "lexer.h"
#include <stdlib.h>
#include "interpreter.h"
#include <string.h>
#include <errno.h>

void process_file(char* filename) {
    FILE* fp = filename == "-" ? stdin : fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Could not open '%s': %s\n",
                filename, strerror(errno));
        return;
    }

    TokenList* tokens = tokenize(fp);
    fclose(fp);

    for (size_t i = 0; i < tokens->len; i++)
        interpret(tokens->head[i]);

    deinit_token_list(tokens);
}

int main(int argc, char** argv) {
    init_interpreter();

    if (argc < 2) {
        process_file("-");
    }
    else for (int i = 1; i < argc; i++)
        process_file(argv[i]);

    deinit_interpreter();
}
