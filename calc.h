#pragma once
#include "num_type.h"
#include <stdlib.h>

typedef struct token_tag {
    N data;
    char type;
} token;

typedef struct token_list_tag {
    token *tokens;
    size_t count;
    int err;
} token_list;

extern char displayed_digits[12];
void format_result(N value, int err);

extern int calc_err;
