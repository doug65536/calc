#pragma once
#include "calc.h"

int read_keyboard(void);
int any_more_input(void);
void reopen_arg(int argc, char const * const * argv);
void show_result(N value, int err);
void dump_token_list(token_list *list);

void *initial_input_buffer(size_t *capacity);

int expand_input_buffer(char **buffer, size_t buffer_count, 
    size_t *buffer_capacity);
void free_token_list(token_list *list);

int initial_output_list(token_list *list, size_t *capacity);
int initial_tokenizer_list(token_list *list, size_t *capacity);
int initial_operators_list(token_list *list, size_t *capacity);
int initial_execution_list(token_list *list, size_t *capacity);
int reserve_token_list(token_list *list, size_t capacity);

#define DIGIT_NEG    10
#define DIGIT_ERR    11
#define DIGIT_EMPTY  12

#if __STDC_HOSTED__
#include <assert.h>
#else
#define assert(e)
#endif