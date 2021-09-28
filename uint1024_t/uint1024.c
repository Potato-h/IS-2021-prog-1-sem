// TODO: Resolve mutability issues

#include "uint1024.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

uint1024_t from_uint(uint32_t x) {
    uint1024_t res;
    memset(&res, 0, sizeof(uint1024_t));
    res.chunk[0] = x;
    return res;
};

uint1024_t add_op(uint1024_t x, uint1024_t y) {
    uint1024_t res = from_uint(0);
    const size_t len = sizeof(res.chunk) / sizeof(uint32_t);
    
    for (size_t i = 0; i < len; i++) {
        uint64_t acc = (uint64_t)x.chunk[i] + (uint64_t)y.chunk[i] + (uint64_t)res.chunk[i];
        res.chunk[i] = acc & 0xFFFFFFFF;
        acc >>= 32;
    
        if (acc != 0)
            res.chunk[i + 1] += acc;  
    }

    return res;
}

static void get_radix(uint1024_t* x, size_t radix) {
    const size_t len = sizeof(x->chunk) / sizeof(uint32_t);
    size_t nonzero;

    for (size_t i = radix + 1; i < len; i++) {
        if (x->chunk[i] != 0) {
            nonzero = i;
            break;
        }
    }

    x->chunk[nonzero]--;

    for (size_t i = radix + 1; i < nonzero; i++) {
        x->chunk[i] = 0xFFFFFFFF;
    }
}

// TODO: rewrite to additional code
uint1024_t subtr_op(uint1024_t x, uint1024_t y) {
    uint1024_t res;
    memcpy(&res, &x, sizeof(uint1024_t));
    const size_t len = sizeof(res.chunk) / sizeof(uint32_t);
    
    for (size_t i = 0; i < len; i++) {
        if (res.chunk[i] < y.chunk[i]) {
            get_radix(&res, i);
            uint64_t acc = (uint64_t)res.chunk[i] + (uint64_t)0xFFFFFFFF + 1 - (uint64_t)y.chunk[i];
            res.chunk[i] = acc & 0xFFFFFFFF; // 2^32 subtract something (> 0) can be hold in uint32_t
        } 
        else {
            res.chunk[i] -= y.chunk[i];
        }
    }

    return res;
}

uint1024_t mult_op(uint1024_t x, uint1024_t y) {
    uint1024_t res = from_uint(0);
    const size_t len = sizeof(res.chunk) / sizeof(uint32_t);

    for (size_t i = 0; i < len; i++) {
        uint1024_t subres = from_uint(0); // calc x * y.chunk[i]

        for (size_t j = 0; j < len; j++) {
            uint64_t acc = (uint64_t)x.chunk[j] * (uint64_t)y.chunk[i] + subres.chunk[j];
            subres.chunk[j] = acc & 0xFFFFFFFF;
            acc >>= 32;

            if (acc != 0)
                subres.chunk[j + 1] += acc;
        }

        memmove(subres.chunk + i, subres.chunk, (len - i) * sizeof(uint32_t)); // shift to i chunks left
        memset(subres.chunk, 0, i * sizeof(uint32_t));
        res = add_op(res, subres);
    }

    return res;
}

struct div_res {
    uint1024_t quot;
    uint32_t rest;
};

static struct div_res div_op(uint1024_t x, uint32_t y) {
    const size_t len = sizeof(x.chunk) / sizeof(uint32_t);
    uint1024_t quot = from_uint(0);
    uint64_t acc = 0;

    for (int i = len - 1; i >= 0; i--) {
        acc <<= 32;
        acc |= x.chunk[i];

        memmove(quot.chunk + 1, quot.chunk, (len - 1) * sizeof(uint32_t)); // shift to 1 chunks left
        memset(quot.chunk, 0, sizeof(uint32_t));
        quot = add_op(quot, from_uint(acc / y));
        acc %= y;
    }

    return (struct div_res){ .quot = quot, .rest = acc };
}

void printf_value(uint1024_t x) {
    const size_t len = sizeof(x.chunk) / sizeof(uint32_t);
    uint1024_t zero = from_uint(0);
    char str[1000];
    size_t i = 0;

    while (memcmp(x.chunk, zero.chunk, len * sizeof(uint32_t)) != 0) {
        struct div_res res = div_op(x, 10);
        x = res.quot;
        str[i++] = res.rest + '0';
    }

    str[i] = '\0';

    for (size_t j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = tmp;
    }

    printf("%s", str);
}

void scanf_value(uint1024_t* x) {
    if (!x) {
        return;
    }    

    *x = from_uint(0);
    char buf[400];
    scanf("%s", buf);
    size_t len = strlen(buf);

    for (size_t i = 0; i < len; i++) {
        *x = mult_op(*x, from_uint(10));
        *x = add_op(*x, from_uint(buf[i] - '0'));
    }
}

int main() {
    uint1024_t x, y;
    scanf_value(&x);
    scanf_value(&y);
    
    uint1024_t res = mult_op(x, y);
    res = subtr_op(res, y);
    res = mult_op(res, x);
    
    printf_value(res);
    printf("\n");

    return 0;
}