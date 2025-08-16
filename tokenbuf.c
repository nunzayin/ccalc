#include "tokenbuf.h"

tokenbuf tokenbuf_init(void) {
    return tokenbuf_t {
        .data = {0},
        .kind = ANYELSE,
        .len = 0;
    };
}
