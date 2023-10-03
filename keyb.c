#include "keyb.h"

static int read_keyboard_cols(void)
{
    /// fixme I/O
    return 0;
}

static void select_keyboard_row(int row)
{
    (void)row;
    /// fixme I/O
}

static int read_keyboard_row(int row)
{
    select_keyboard_row(row);
    return read_keyboard_cols();
}

static char const keyboard_matrix[][4] = {
    {'7', '8', '9', '/'},
    {'4', '5', '6', '+'},
    {'1', '2', '3', '-'},
    {'0', 'C', '=', '+'}
};

static int read_keyboard_iter(void)
{
    char press = 0;
    for (int row = 0; row < 4; ++row) {
        int bits = read_keyboard_row(row);

        for (unsigned mask = 1, bit = 0; 
                bits && mask < 0x10;
                (mask <<= 1), ++bit) {
            if (bits & mask) {
                if (press)
                    return 0;
                press = keyboard_matrix[row][bit];
            }
        }
    }
    return press;
}

int read_keyboard()
{
    char last_press = 0;
    for (int count = 0; count < KEYB_DEBOUNCE_ITER; ++count) {
        int press = read_keyboard_iter();
        if (press != last_press)
            count = 0;
        last_press = press;
    }
    return last_press;
}

int any_more_input()
{
    return 1;
}

void reopen_arg(int argc, char const * const * argv)
{
    (void)argc;
    (void)argv;
}

void abort()
{
    while(1) {
        __asm__ __volatile__ ("nop");
    }
}
