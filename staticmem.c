// Statically allocated memory for firmware
#include "io.h"

static char input_buffer[512];

void *initial_input_buffer(size_t *capacity)
{
    *capacity = sizeof(input_buffer);
    return input_buffer;
}

int expand_input_buffer(char **buffer, size_t buffer_count, 
    size_t *buffer_capacity)
{
    (void)buffer;
    (void)buffer_count;
    (void)buffer_capacity;
    return 0;
}

void free_token_list(token_list *list)
{    
    list->tokens = 0;
}

int reserve_token_list(token_list *list, size_t capacity)
{
    (void)list;
    (void)capacity;
    return 0;   
}

static token output_tokens[128];

int initial_output_list(token_list *list, size_t *output_capacity)
{
    *output_capacity = sizeof(output_tokens) / sizeof(*output_tokens);
    list->tokens = output_tokens;
    return 1;
}

static token tokenizer_tokens[128];

int initial_tokenizer_list(token_list *list, size_t *capacity)
{
    *capacity = sizeof(tokenizer_tokens) / sizeof(*tokenizer_tokens);
    list->tokens = tokenizer_tokens;
    return 1;
}

static token operators_tokens[128];

int initial_operators_list(token_list *list, size_t *capacity)
{
    *capacity = sizeof(operators_tokens) / sizeof(*operators_tokens);
    list->tokens = operators_tokens;
    return 1;
}

static token execution_tokens[128];

int initial_execution_list(token_list *list, size_t *capacity)
{
    *capacity = sizeof(execution_tokens) / sizeof(*execution_tokens);
    list->tokens = execution_tokens;
    return 1;
}
