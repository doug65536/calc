#pragma once

#include <stdlib.h>
#include <stdint.h>

int isdigit(int c);
int isspace(int c);
void *memcpy(void *d, void const *s, size_t n);
void *memset(void *d, int v, size_t n);
char *strchr(char const *chars, int ch);
size_t strspn(char const *s, char const *allowed);


typedef struct divmod_tag {
    int64_t quot;
    int64_t rem;
    int err;
} divmod;

typedef struct udivmod_tag {
    uint64_t quot;
    uint64_t rem;
    int err;
} udivmod;

divmod signed_divmod(int64_t signed_numerator, int64_t signed_denominator);
udivmod unsigned_divmod(uint64_t numerator, uint64_t denominator);
