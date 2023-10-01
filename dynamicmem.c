// Dynamically allocated memory if your environment is that elaborate
#include "io.h"
#include "freestand.h"

void *initial_input_buffer(size_t *capacity)
{
    return malloc(*capacity);
}

int reserve_token_list(token_list *list, size_t capacity)
{
    // Allocate a new buffer
    token *new_buffer = (token*)malloc(capacity * sizeof(token));

    if (new_buffer) {
        // Copy everything into it
        memcpy(new_buffer, list->tokens, list->count * sizeof(*list->tokens));

        // Free the old buffer
        free(list->tokens);

        // Attach to the new buffer
        list->tokens = new_buffer;
    }
    return 1;
}

int expand_input_buffer(char **buffer, size_t buffer_count, 
    size_t *buffer_capacity)
{
    *buffer_capacity += *buffer_capacity;
    char *new_buffer = malloc(*buffer_capacity);
    if (!new_buffer)
        return 0;
    memcpy(new_buffer, *buffer, buffer_count);
    free(*buffer);
    *buffer = new_buffer;
    return 1;
}

void free_token_list(token_list *list)
{
    free(list->tokens);
    list->tokens = 0;
}

int initial_tokenizer_list(token_list *list, size_t *capacity)
{
    return reserve_token_list(list, *capacity);
}

int initial_operators_list(token_list *list, size_t *capacity)
{
    return reserve_token_list(list, *capacity);
}

int initial_output_list(token_list *list, size_t *capacity)
{
    return reserve_token_list(list, *capacity);
}

int initial_execution_list(token_list *list, size_t *capacity)
{
    return reserve_token_list(list, *capacity);
}
