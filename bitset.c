/**
 * @file bitset.c
 * @author Jason Conway (jpc@jasonconway.dev)
 * @brief Bitset implementation for `frappe`
 * @version 23.03
 * @date 2023-03-08
 *
 * @copyright Copyright (c) 2023-2025 Jason Conway. All rights reserved.
 *
 */

#include "bitset.h"

#pragma region Allocator

static void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (unlikely(!ptr)) {
        abort();
    }
    return ptr;
}

static void *xcalloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    if (unlikely(!ptr)) {
        abort();
    }
    return ptr;
}

static void *xrealloc(void *ptr, size_t size)
{
    void *ptr = realloc(ptr, size);
    if (unlikely(!ptr)) {
        abort();
    }
    return ptr;
}

static void xfree(void *ptr)
{
    free(ptr);
}

#pragma endregion


bitset_t *bitset_new(size_t elements)
{
    size_t w = REQ_SZ(elements);
    bitset_t *set = xcalloc(w, sizeof(bitset_t));
    bitset_set_size(set, w - 1);
    return set;
}

void bitset_free(bitset_t *set)
{
    free(set);
}

bool bitset_get(const bitset_t *set, size_t index)
{
    const size_t word = ELEM_WORD(index);
    const size_t bit = ELEM_BIT(index);
    return set[word] & (1ull << bit);
}

void bitset_set(bitset_t *set, size_t index)
{
    const size_t word = ELEM_WORD(index);
    const size_t bit = ELEM_BIT(index);
    set[word] |= (1ull << bit);
}

void bitset_clear(bitset_t *set, size_t index)
{
    const size_t word = ELEM_WORD(index);
    const size_t bit = ELEM_BIT(index);
    set[word] &= ~(1ull << bit);
}

bitset_t *bitset_dupl(const bitset_t *set)
{
    const size_t len = BITSET_FIELD(set, size) + 1;
    bitset_t *dupl = xmalloc(len * sizeof(bitset_t));
    return memcpy(dupl, set, len * sizeof(bitset_t));
}

bitset_t *bitset_copy(bitset_t *dst, const bitset_t *src)
{
    const size_t len = BITSET_FIELD(src, size);
    return memcpy(dst, src, sizeof(bitset_t) * (len + 1));
}

bitset_t *bitset_null(bitset_t *set, size_t elements)
{
    const size_t w = REQ_SZ(elements);
    memset(set, 0, sizeof(bitset_t) * w);
    bitset_set_size(set, w - 1);
    return set;
}

bitset_t *bitset_reset(bitset_t *set)
{
    const size_t w = BITSET_FIELD(set, size);
    memset(set, 0, (w + 1) * sizeof(bitset_t));
    bitset_set_size(set, w);
    return set;
}

bitset_t *bitset_universe(bitset_t *set, size_t elements)
{
    set[0] = 0;
    size_t w = REQ_SZ(elements) - 1;
    for (size_t i = 1; i <= w; i++) {
        set[i] = UINT64_MAX;
    }
    set[w] >>= (64 * w - elements);
    bitset_set_size(set, w);
    return set;
}

bool bitset_equal(const bitset_t *a, const bitset_t *b)
{
    const size_t len = BITSET_FIELD(a, size);
    return !memcmp(&a[1], &b[1], len * sizeof(bitset_t));
}

size_t bitset_ord(const bitset_t *set)
{
    size_t sum = 0;
    for (size_t i = BITSET_FIELD(set, size); i > 0; i--) {
        sum += __builtin_popcountll(set[i]);
    }
    return sum;
}

void bitset_tag_ord(bitset_t *set)
{
    size_t sum = 0;
    for (size_t i = BITSET_FIELD(set, size); i > 0; i--) {
        sum += __builtin_popcountll(set[i]);
    }
    bitset_set_tag(set, sum);
}

size_t bitset_dist(const bitset_t *restrict a, const bitset_t *restrict b)
{
    size_t sum = 0;
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        sum += __builtin_popcountll(a[i] & b[i]);
    }
    return sum;
}

bitset_t *bitset_and(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_COPY_FIELD(r, a, size); i > 0; i--) {
        r[i] = a[i] & b[i];
    }
    return r;
}

void bitset_and_inplace(bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        a[i] &= b[i];
    }
}

bitset_t *bitset_or(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_COPY_FIELD(r, a, size); i > 0; i--) {
        r[i] = a[i] | b[i];
    }
    return r;
}

void bitset_or_inplace(bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        a[i] |= b[i];
    }
}

void bitset_xand(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        r[i] &= a[i] ^ b[i];
    }
}

bitset_t *bitset_diff(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_COPY_FIELD(r, a, size); i > 0; i--) {
        r[i] = a[i] & ~b[i];
    }
    return r;
}

void bitset_diff_inplace(bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        a[i] &= ~b[i];
    }
}

void bitset_diff_inplace_alt(const bitset_t *restrict a, bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        b[i] = a[i] & ~b[i];
    }
}

bitset_t *bitset_symmetric_diff_union(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b, const bitset_t *restrict c)
{
    for (size_t i = BITSET_COPY_FIELD(r, a, size); i > 0; i--) {
        r[i] = a[i] | (b[i] & ~c[i]);
    }
    return r;
}

void bitset_mask_inplace(bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        a[i] &= ~(a[i] & ~b[i]);
    }
}

bool bitset_empty(const bitset_t *a)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        if (a[i]) {
            return false;
        }
    }
    return true;
}

bitset_t *bitset_merge(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b, const bitset_t *restrict c)
{
    for (size_t i = BITSET_COPY_FIELD(r, a, size); i > 0; i--) {
        // Equivalent to the idiomatic:
        // `r[i] = (a[i] & c[i]) | (b[i] & ~c[i]);`
        // but with fewer instructions
        r[i] = b[i] ^ ((b[i] ^ a[i]) & c[i]);
    }
    return r;
}

bool bitset_disjoint(const bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        if (a[i] & b[i]) {
            return false;
        }
    }
    return true;
}

bool bitset_implies(const bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        if (a[i] & ~b[i]) {
            return false;
        }
    }
    return true;
}

bool bitset_implies_alt(const bitset_t *restrict a, const bitset_t *restrict b)
{
    for (size_t i = BITSET_FIELD(a, size); i > 0; i--) {
        if (~a[i] & b[i]) {
            return false;
        }
    }
    return true;
}
