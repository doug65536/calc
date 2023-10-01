#include "num_type.h"
#include "calc.h"
#include "io.h"
#include "freestand.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define N_FMT PRId64

int read_keyboard()
{
    int c = getchar();
    return c;
}

static void render_display()
{
    for (size_t i = 0; i < sizeof(displayed_digits); ++i) {
        if (displayed_digits[i] < 10)
            printf("%c", '0' + displayed_digits[i]);
        else if (displayed_digits[i] == DIGIT_NEG)
            printf("-");
        else if (displayed_digits[i] == DIGIT_ERR)
            printf("E");
        else if (displayed_digits[i] == DIGIT_EMPTY)
            printf(" ");
    }
    printf("\n");
}

void show_result(N value, int err)
{
    format_result(value, err);
    render_display();
}

int any_more_input()
{
    return !feof(stdin);
}

void reopen_arg(int argc, char const * const * argv)
{
    if (argc > 1)
        freopen(argv[1], "rt", stdin);

}

void dump_token_list(token_list *list)
{
    fprintf(stderr, "{ ");
    for (size_t i = 0; i < list->count; ++i) {
        char type = list->tokens[i].type;
        N data = list->tokens[i].data;
        switch (type) {
        case 'o':
            fprintf(stderr, "%c ", (char)data);
            break;
        case 'n':
            fprintf(stderr, "%" N_FMT " ", data);
            break;
        }
    }
    fprintf(stderr, "}\n");
}
