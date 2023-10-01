
#include <stdint.h>
#include "seg7.h"

//     a
//   ----- 
// f|     |b
//  |  g  |
//   ----- 
//  |     |
// e|  d  |c
//   ----- 
//

#define SEG7_DIGIT(a, b, c, d, e, f, g) \
    ((a)|((b)<<1)|((c)<<2)|((d)<<3)|((e)<<4)|((f)<<5)|((g)<<6))

static uint8_t const seg7_lookup[13] = {
    SEG7_DIGIT(1,1,1,1,1,1,0),  // 0
    SEG7_DIGIT(0,1,1,0,0,0,0),  // 1
    SEG7_DIGIT(1,1,0,1,1,0,1),  // 2
    SEG7_DIGIT(1,1,1,1,0,0,1),  // 3 
    SEG7_DIGIT(0,1,1,0,0,1,1),  // 4
    SEG7_DIGIT(1,0,1,1,0,1,1),  // 5
    SEG7_DIGIT(1,0,1,1,1,1,1),  // 6
    SEG7_DIGIT(1,1,1,0,0,0,0),  // 7
    SEG7_DIGIT(1,1,1,1,1,1,1),  // 8
    SEG7_DIGIT(1,1,1,1,0,1,1),  // 9

    SEG7_DIGIT(0,0,0,0,0,0,1),  // 10 (displays -)
    SEG7_DIGIT(1,0,0,1,1,1,1),  // 11 (displays E)
    SEG7_DIGIT(0,0,0,0,0,0,0),  // 12 (displays nothing)
};

// Use SEG7_* constant
static void change_digit(int digit)
{
    uint8_t bits = (digit >= 0 && 
            (unsigned)digit < sizeof(seg7_lookup))
        ? seg7_lookup[digit]
        : 0;
    (void)bits;
    (void)digit;
    /// fixme I/O
}

// -1 means no digit
static void select_digit(int digit)
{
    (void)digit;
    /// fixme I/O
}

static unsigned delay_loop(uint64_t loops)
{
    unsigned volatile dummy = 0;
    while (loops--)
        ++dummy;
    return dummy;
}

static void drive_digit(int digit, int index)
{
    // Select no digits
    select_digit(-1);
    delay_loop(DIGIT_SELECT_LOOPS);

    // Change the segments
    change_digit(digit);
    delay_loop(DIGIT_SETUP_LOOPS);

    // Select the new digit
    select_digit(index);
    delay_loop(DIGIT_HOLD_LOOPS);
}

static void render_display(void)
{
    for (size_t i = 0; i < sizeof(displayed_digits); ++i)
        drive_digit(displayed_digits[i], i);
}

void show_result(N value, int err)
{
    format_result(value, err);
    render_display();
}

void dump_token_list(token_list *list)
{
    (void)list;
}
