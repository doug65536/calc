#include "calc.h"
#include <stdlib.h>
#include <stdint.h>
#include "num_type.h"
#include "freestand.h"
// #include "seg7.h"
// #include "keyb.h"
#include "io.h"

int calc_err;
char displayed_digits[12];

void format_result(N value, int err)
{
    // Clear digits
    memset(displayed_digits, DIGIT_EMPTY, sizeof(displayed_digits));

    char *out = displayed_digits + sizeof(displayed_digits);

    if (!err) {
        int neg = 0;

        if (value < 0) {
            neg = 1;
            value = -value;
        }

        do {
            udivmod dmr = unsigned_divmod(value, 10);
            *--out = dmr.rem;
            value = dmr.quot;
        } while (value);

        if (neg)
            *--out = DIGIT_NEG;
    } else {
        // E
        *--out = DIGIT_ERR;
    }
}

static char const * const valid_operators = "+-*/()N";
static char const * const valid_operators_noparen = "+-*/N";
static char const * const unary_ops = "N";

static N parse_digits(char const *s)
{
    N value = 0;
    int neg = (*s == '-');
    s += neg;
    while (isdigit(*s)) {
        value *= 10;
        value += *s - '0';
        ++s;
    }
    if (neg)
        value = -value;
    return value;
}

static int precedence_from_op(int op)
{
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    if (op == 'N')
        return 3;
    return 0;
}

static int precedence_is_higher(int op1, int op2)
{
    return precedence_from_op(op1) > precedence_from_op(op2);
}


static token *pop_token_list(token_list *list)
{
    return &list->tokens[--list->count];
}

static token *peek_token_list(token_list const *list)
{
    return list->count
        ? &list->tokens[list->count - 1]
        : 0;
}

static token *push_token_list(token_list *list, size_t *capacity)
{
    if (list->count == *capacity) {
        *capacity += *capacity;
        if (!reserve_token_list(list, *capacity))
            return 0;
    }
    
    token *result = list->tokens + list->count;
    
    ++list->count;

    return result;
}

static void copy_token(token *d, token const *s)
{
    d->data = s->data;
    d->type = s->type;
}

static void move_token(token_list *popped, 
    token_list *pushed, size_t *pushed_capacity)
{
    token *popped_token = pop_token_list(popped);
    token *pushed_token = push_token_list(pushed, pushed_capacity);
    copy_token(pushed_token, popped_token);
}

static token_list tokenize(char const *st, char const *en)
{
    size_t capacity = 4;
    token_list result = {};
    initial_tokenizer_list(&result, &capacity);

    while (st < en) {
        if (isspace(*st)) {
            ++st;
            continue;
        }

        token *new_token = push_token_list(&result, &capacity);
        
        // Find out how many consecutive digits exist at the start point
        size_t accepted_digits = strspn(st, "0123456789");

        if (accepted_digits && accepted_digits < 10) {
            // Parse number
            char n[10];
            // Copy digits into buffer
            memcpy(n, st, accepted_digits);
            // Null terminate the buffer
            n[accepted_digits] = 0;
            // Advance past accepted number
            st += accepted_digits;
            // Parse the null terminated string in the buffer
            int value = parse_digits(n);
            // Add a token to the array
            new_token->data = value;
            new_token->type = 'n';
        } else if (strchr(valid_operators, *st)) {
            // Parse operator

            // Special case unary negation
            if (*st != '-' || (result.count >= 2 &&
                    result.tokens[result.count-2].type != 'o')) {
                // Normal operator
                new_token->data = *st;
                new_token->type = 'o';
            } else {
                // Make it N operator (unary negation)
                new_token->data = 'N';
                new_token->type = 'o';
            }
            st += 1;
        } else {
            pop_token_list(&result);
            result.err = 1;
            break;
        }
    }

    return result;
}

static token_list shunting_yard(token_list const *infix_tokens)
{
    token_list output = {};
    size_t output_capacity = 4;
    if (!initial_output_list(&output, &output_capacity)) {
        calc_err = 1;
        return output;
    }

    token_list operators = {};
    size_t operators_capacity = 4;
    initial_operators_list(&operators, &operators_capacity);
    
    for (size_t i = 0; i < infix_tokens->count; ++i) {
        token const* t = infix_tokens->tokens + i;
        assert(t->type == 'o' || t->type == 'n');

        if (t->type == 'n') {
            token *output_token = push_token_list(&output, &output_capacity);
            copy_token(output_token, t);
        } else if (strchr(valid_operators_noparen, t->data)) {
            // Stop if there are operators
            while (operators.count) {
                // Peek at the operator it would pop in operators
                token const *last_operator_token = peek_token_list(&operators);
                int last_operator = last_operator_token->data;

                // Stop if it is a closing parenthesis
                if (!strchr(valid_operators_noparen, last_operator))
                    break;
                
                // Stop if it is lower or equal precedence versus this operator
                if (!precedence_is_higher(last_operator, t->data))
                    break;

                // Pop the operator and push it to output
                move_token(&operators, &output, &output_capacity);
            }

            token *pushed_oper = push_token_list(
                &operators, &operators_capacity);
            
            copy_token(pushed_oper, t);
        } else if (t->data == '(') {
            token *pushed_oper = push_token_list(
                &operators, &operators_capacity);
            copy_token(pushed_oper, t);
        } else if (t->data == ')') {
            // Move all of the operators in the parentheses to output
            while (operators.count &&
                peek_token_list(&operators)->data != '(') {
                move_token(&operators, &output, &output_capacity);
            }
            pop_token_list(&operators);
        }
    }

    while (operators.count)
        move_token(&operators, &output, &output_capacity);

    return output;
}

static int64_t number_add(N const *lhs_val, N const *rhs_val)
{
    return *lhs_val + *rhs_val;
}

static int64_t number_sub(N const *lhs_val, N const *rhs_val)
{
    return *lhs_val - *rhs_val;
}

static int64_t number_mul(N const *lhs_val, N const *rhs_val, 
    uint8_t use_signed)
{
#ifndef BIG_UC
    unsigned char const *lhs_bytes = (unsigned char const *)lhs_val;
    unsigned char const *rhs_bytes = (unsigned char const *)rhs_val;

    N total = 0;
    unsigned char *total_bytes = (unsigned char *)&total;

    // Convert 0/1 to all zeros or all ones
    use_signed = -use_signed;

    // Do it like elementary school, in 8x8=16 bit chunks
    for (uint8_t bp = 0; bp < sizeof(N); ++bp) {
        for (uint8_t tp = 0; tp < sizeof(N); ++tp) {
            uint8_t p = bp + tp;
            if (p >= sizeof(total))
                break;
            uint16_t sub_product = lhs_bytes[tp] * rhs_bytes[bp];
            uint8_t oldv = total_bytes[p];
            uint8_t newv = oldv + (sub_product & 0xFF);
            uint8_t c = newv < oldv;
            ++p;
            oldv = total_bytes[p];
            newv = oldv + (sub_product >> 8) + c;
            total_bytes[p] = newv;
            ++p;
            uint8_t sign_extend = use_signed & -(sub_product >> 15);
            for (c = newv < oldv; (c || sign_extend) && 
                    p < sizeof(total); ++p) {
                oldv = total_bytes[p];
                newv = oldv + sign_extend + c;
                c = newv < oldv;
                total_bytes[p] = newv;
            }
        }
    }
    return total;
#else
    return *lhs_val * *rhs_val;
#endif
}

static int64_t number_div(N const *lhs_val, N const *rhs_val)
{
    if (*rhs_val != 0) {
        divmod result = signed_divmod(
            *lhs_val, *rhs_val);
        if (!result.err) {
            return result.quot;
        } else {
            calc_err = 1;
        }
    } else{
        calc_err = 1;
    }
    return INT64_MAX;
}

static N execute_rpn(token_list const *rpn_tokens, int *err)
{
    *err = 0;

    token_list execution_stack = {};
    size_t execution_stack_capacity = 4;
    initial_execution_list(&execution_stack, &execution_stack_capacity);

    for (size_t i = 0; i < rpn_tokens->count; ++i) {
        token *t = rpn_tokens->tokens + i;
        if (t->type == 'n') {
            token *pushed_token = push_token_list(
                &execution_stack, &execution_stack_capacity);
            copy_token(pushed_token, t);
        } else {
            token *rhs_val = pop_token_list(&execution_stack);
            token *lhs_val = 0;
            
            if (!strchr(unary_ops, t->data))
                lhs_val = pop_token_list(&execution_stack);

            N value;
            switch ((char)t->data) {
                case '+':
                    value = number_add(&lhs_val->data, &rhs_val->data);
                    break;
                case '-':
                    value = number_sub(&lhs_val->data, &rhs_val->data);
                    break;
                case '*':
                    value = number_mul(&lhs_val->data, &rhs_val->data);
                    break;
                case '/':
                    value = number_div(&lhs_val->data, &rhs_val->data);
                    break;
            
                case 'N':
                    value = -rhs_val->data;
                    break;                    
            }

            token *sub = push_token_list(
                &execution_stack, &execution_stack_capacity);
            
            sub->data = value;
            sub->type = 'n';
        }
    }

    if (execution_stack.count != 1) {
        *err = 1;
        return 0;
    }

    N value = execution_stack.tokens[0].data;

    free_token_list(&execution_stack);

    return value;
}

int main(int argc, char const * const * argv)
{
    // Let the I/O layer redirect stdin to point to test file on command line
    // or do nothing on firmware
    reopen_arg(argc, argv);
    
    size_t buffer_capacity = 4;
    char *buffer = initial_input_buffer(&buffer_capacity);
    size_t buffer_count = 0;
    while (any_more_input()) {        
        int press = read_keyboard();

        if (press == -1 || press == '\n' || press == '=')
            break;
        
        if (press == 'c') {
            buffer_count = 0;
            calc_err = 0;
            continue;
        }
        
        // Stuck until 'c' if calc_err
        if (calc_err && press != 'c')
            continue;
        
        if (!isdigit(press) && !strchr(valid_operators, press))
            continue;
            
        if (buffer_count == buffer_capacity) {
            if (!expand_input_buffer(&buffer, 
                    buffer_count, 
                    &buffer_capacity)) {
                calc_err = 1;
                continue;
            }
        }
        buffer[buffer_count++] = press;            

        token_list infix_tokens = tokenize(buffer, buffer + buffer_count);

        dump_token_list(&infix_tokens);

        token_list rpn_tokens = shunting_yard(&infix_tokens);

        dump_token_list(&rpn_tokens);

        int err;
        N result = execute_rpn(&rpn_tokens, &err);

        show_result(result, err);

        free_token_list(&infix_tokens);
        free_token_list(&rpn_tokens);
    }

    return 0;
}
