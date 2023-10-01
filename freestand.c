#include "freestand.h"

int isdigit(int c)
{
    return c >= 0 && c <= '9';
}

int isspace(int c)
{
    return c <= ' ' && c >= 0;
}

void *memcpy(void *d, void const *s, size_t n)
{
    char *dc = (char *)d;
    char const *sc = (char const *)s;
    for (size_t i = 0; i < n; ++i)
        dc[i] = sc[i];
    return d;
}

void *memset(void *d, int v, size_t n)
{
    unsigned char *dc = (unsigned char *)d;
    for (size_t i = 0; i < n; ++i)
        dc[i] = (unsigned char)v;
    return d;
}

char *strchr(char const *chars, int ch)
{
    for (size_t i = 0; !ch || chars[i]; ++i) {
        if ((unsigned char)chars[i] == (unsigned char)ch)
            return (char*)(chars + i);
    }
    return 0;
}

size_t strspn(char const *s, char const *allowed)
{
    size_t i;
    for (i = 0; s[i]; ++i) {
        if (!strchr(allowed, (unsigned char)s[i]))
            break;
    }
    return i;
}

#ifdef HAVE_LIBGCC
divmod signed_divmod(int64_t numerator, int64_t denominator)
{
    divmod r = { 
        denominator ? numerator / denominator : 0,
        denominator ? numerator % denominator : 0,
        !denominator
    };
    return r;
}
udivmod unsigned_divmod(uint64_t numerator, uint64_t denominator)
{
    udivmod r = {
        denominator ? numerator / denominator : 0,
        denominator ? numerator % denominator : 0,
        !denominator
    };
    return r;
}
#else
udivmod unsigned_divmod(uint64_t numerator, uint64_t denominator)
{
    udivmod result = { 0, 0, 0 };

    if (denominator != 0) {
        for (int i = 0; i < 64; ++i) {
            unsigned c = (int64_t)numerator < 0;
            numerator <<= 1;
            result.rem = (result.rem << 1) + c;
            c = (result.rem >= denominator);
            result.rem = result.rem - (denominator & -c);
            result.quot = (result.quot << 1) + c;
        }
    } else {
        result.err = 1;
    }

    return result;
}

divmod signed_divmod(int64_t signed_numerator, int64_t signed_denominator)
{
    int neg_numer = (signed_numerator < 0);
    int neg_denom = (signed_denominator < 0);
    uint64_t numerator = signed_numerator ^ -neg_numer;
    numerator += neg_numer;
    int sign = neg_numer ^ neg_denom;
    uint64_t denominator = signed_denominator ^ -neg_denom;
    denominator += neg_denom;

    udivmod u = unsigned_divmod(numerator, denominator);

    divmod s;
    s.rem = u.rem ^ -neg_numer;
    s.rem += neg_numer;
    s.quot = u.quot ^ -sign;
    s.quot += sign;

    return s;
}
#endif
