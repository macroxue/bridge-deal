#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////
// BIG integer
//////////////////////////////////////////////////
#define BIG_SIZE   6
#define BASE       10000
#define BASE_LOG   4

typedef int big_t[BIG_SIZE];

void big_set(big_t r, int n)
{
    int i;
    assert(n < BASE);
    for (i = 0; i < BIG_SIZE; i++) r[i] = 0;
    r[0] = n;
}

void big_add(big_t r, big_t m, big_t n)
{
    int i, c = 0;
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] = m[i] + n[i] + c;
        c = (r[i] >= BASE);
        if (c) r[i] -= BASE;
    }
    assert(c == 0);
}

void big_sub(big_t r, big_t m, big_t n)
{
    int i, c = 0;
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] = m[i] - n[i] - c;
        c = r[i] < 0;
        if (c) r[i] += BASE;
    }
    assert(c == 0);
}

int
big_cmp(big_t m, big_t n)
{
    int i, c = 0, z = 1, r;
    for (i = 0; i < BIG_SIZE; i++) {
        r = m[i] - n[i] - c;
        c = r < 0;
        if (r) z = 0;
    }
    if (c == 1) return -1;
    else if (z) return 0;
    else return 1;
}

void big_mul_i(big_t r, big_t m, int n)
{
    int i, c = 0;
    assert(n < BASE);
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] = m[i] * n + c;
        c = r[i] / BASE;
        r[i] -= c*BASE;
    }
    assert(c == 0);
}

void big_mul(big_t r, big_t m, big_t n)
{
    int i, j, c = 0;

    assert(r != m && r != n);
    big_set(r, 0);
    for (i = 0; i < BIG_SIZE; i++) {
        for (j = 0; j < BIG_SIZE; j++) {
            r[i+j] += m[i] * n[j];
        }
    }
    c = 0;
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] += c;
        c = r[i] / BASE;
        r[i] -= c*BASE;
    }
    assert(c == 0);
}

void big_div_i(big_t r, big_t m, int n)
{
    int i, c = 0, d;
    assert(n < BASE);
    for (i = BIG_SIZE - 1; i >= 0; i--) {
        d = c * BASE + m[i];
        r[i] = d / n;
        c = d - r[i] * n;
    }
    assert(c == 0);
}

void big_choose(big_t r, int m, int n)
{
    int i;
    big_set(r, 1);
    for (i = m-n+1; i <= m; i++)
        big_mul_i(r, r, i);
    for (i = 1; i <= n; i++)
        big_div_i(r, r, i);
}

void big_rand(big_t r, big_t t)
{
    int i;
    for (i = BIG_SIZE-1; i >= 0 && t[i] == 0; i--);

    big_set(r, 0);
    r[i] = rand() % (t[i]+1);
    for (i--; i >= 0; i--) {
        r[i] = rand() % BASE;
    }
    while (big_cmp(r, t) >= 0) {
        big_sub(r, r, t);
    }
}

void big_show(big_t r)
{
    int i;
    for (i = BIG_SIZE - 1; i >= 0 && r[i] == 0; i--);
    if (i >= 0) {
        printf("%d", r[i]);
        for (i--; i >= 0; i--)
            printf("%04d", r[i]);
    } else {
        printf("0");
    }
}

void big_read(big_t r, char *s)
{
    int i, l;

    big_set(r, 0);

    l = strlen(s);
    for (i = l-1; i >= 0; i--) {
        r[i/BASE_LOG] = r[i/BASE_LOG]*10 + s[l-1-i] - '0';
    }
}
