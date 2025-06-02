/**
 * @file bitset.h
 * @author Jason Conway (jpc@jasonconway.dev)
 * @brief Bitset implementation for `frappe`
 * @version 23.03
 * @date 2023-03-08
 *
 * @copyright Copyright (c) 2023-2025 Jason Conway. All rights reserved.
 *
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __has_builtin
    #define __has_builtin(x)
#endif

#ifndef __has_attribute
    #define __has_attribute(x)
#endif

#ifndef unlikely
    #if __has_builtin(__builtin_expect)
        #define unlikely(x) __builtin_expect(!!(x), 0)
    #else
        #define unlikely(x)
    #endif
#endif

#ifndef likely
    #if __has_builtin(__builtin_expect)
        #define likely(x) __builtin_expect(!!(x), 1)
    #else
        #define likely(x)
    #endif
#endif

#ifndef __packed
    #if __has_attribute(packed)
        #define __packed __attribute__((packed))
    #else
        #error "Compiler must support 'packed' attribute"
    #endif
#endif


// The various state-indicating flags for individual bitsets
typedef enum setflags_t {
    SET_ACTIVE = 0x01,
    SET_COMPRESSED = 0x02,
    SET_CAN_BE_FREED = 0x04,
} setflags_t;

// Description of the first 64-bits of every bitset
typedef struct __packed setfields_t {
    uint32_t size;  // Number of data-containing words in a set
    uint16_t flags; // Optional field used for flags
    uint16_t tag;   // Optional field used to tag to a bitset for sorting
} setfields_t;

typedef uint64_t bitset_t;

// Return true if the data in set `a` equals the data in set `b`
bool bitset_equal(const bitset_t *a, const bitset_t *b);

// Create a new bitset capable of holding `elements` number of bits
bitset_t *bitset_new(size_t elements);

// Free allocated set memory
void bitset_free(bitset_t *set);

// Make `set` the empty set of `size` elements
bitset_t *bitset_null(bitset_t *set, size_t size);

// Turn the existing set, `set`, into an empty set
bitset_t *bitset_reset(bitset_t *set);

// Make `set` the universal set of `size` elements
bitset_t *bitset_universe(bitset_t *set, size_t size);

// Create a duplicate copy of an existing set
bitset_t *bitset_dupl(const bitset_t *set);

// Copy the contents of `src` to `dst`
bitset_t *bitset_copy(bitset_t *dst, const bitset_t *src);

// Compute the difference (relative complement) of `a` and `b`, storing the result in `r`
bitset_t *bitset_diff(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b);

// Compute the difference (relative complement) of `a` and `b`, storing the result in `a`
//     `a[i] = a[i] & ~b[i]`
void bitset_diff_inplace(bitset_t *restrict a, const bitset_t *restrict b);

// Compute the difference (relative complement) of `a` and `b`, storing the result in `b`
//     `b[i] = a[i] & ~b[i]`
void bitset_diff_inplace_alt(const bitset_t *restrict a, bitset_t *restrict b);

// Compute the union of `a` with the set difference of `b` and `c`, storing the result in `r`
//     `r[i] = a[i] | (b[i] & ~c[i])`
bitset_t *bitset_symmetric_diff_union(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b, const bitset_t *restrict c);

// Compute the set difference of `a` with the difference of sets `a` and `b`, storing the result in `a`
//     `a[i] &= ~(a[i] & ~b[i])`
// Clears the bits in `a` where `b` is 0, and retains the bits in `a` where `b` is 1
void bitset_mask_inplace(bitset_t *restrict a, const bitset_t *restrict b);

// Compute the union of sets `a` and `b`, storing the result in `r`
bitset_t *bitset_or(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b);

// Compute the union of sets `a` and `b`, storing the result in `a`
void bitset_or_inplace(bitset_t *restrict a, const bitset_t *restrict b);

// Compute the intersection of sets `a` and `b`, storing the result in `r`
bitset_t *bitset_and(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b);

// Compute the intersection of sets `a` and `b`, storing the result in `a`
void bitset_and_inplace(bitset_t *restrict a, const bitset_t *restrict b);

// Compute the intersection of `r` with the exclusive disjunction (XOR) of sets `a` and `b`
// `r[i] &= a[i] ^ b[i]`
void bitset_xand(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b);

// Return true if set `a` is empty
bool bitset_empty(const bitset_t *a);

// Compute the number of set elements (number of 'ones') in the set
size_t bitset_ord(const bitset_t *set);

// Compute the distance (# elements in common) between `a` and `b`
size_t bitset_dist(const bitset_t *restrict a, const bitset_t *restrict b);

// Compute the union of the intersection of sets `a` and `c` with set difference between `b` and `c`
// storing the result in `r`
bitset_t *bitset_merge(bitset_t *restrict r, const bitset_t *restrict a, const bitset_t *restrict b, const bitset_t *restrict c);

// Return true if the intersection of `a` and `b` is empty
bool bitset_disjoint(const bitset_t *restrict a, const bitset_t *restrict b);

// Return true if `a` implies `b` (`b` contains `a`)
bool bitset_implies(const bitset_t *restrict a, const bitset_t *restrict b);

// Return true if `b` implies `a` (`a` contains `b`)
bool bitset_implies_alt(const bitset_t *restrict a, const bitset_t *restrict b);

// Set the ith bit
void bitset_set(bitset_t *set, size_t i);

// Clear the ith bit
void bitset_clear(bitset_t *set, size_t i);

// Get the ith bit
bool bitset_get(const bitset_t *set, size_t i);

// Set the `tag` field to the number of 'ones' in the bitset
void bitset_tag_ord(bitset_t *set);

// Returns an lvalue to field
#define BITSET_FIELD(set, field) \
    (((setfields_t *)(set))->field)

// Copy `field` from `src` to `dst`
#define BITSET_COPY_FIELD(dst, src, field) \
    (((setfields_t *)(dst))->field) = (((setfields_t *)(src))->field)

// Returns the index of the word containing bit`elem`
#define ELEM_WORD(elem) \
    (1 + ((elem) >> 6))

// Returns the index of bit `elem` within a word
#define ELEM_BIT(elem) \
    ((elem) & 63)

// Returns the bitset size required to store `elems` bits
#define REQ_SZ(elems) \
    (2 + (((elems) - 1) >> 6))

// Return the number of 64-bit words in the bitset
#define bitset_get_size(set) \
    (((setfields_t *)(set))->size)

// Store the number of 64-bit words in the bitset
#define bitset_set_size(set, val) \
    (((setfields_t *)(set))->size = (uint32_t)(val))

// Test for flag `flag`, returning true if set
#define bitset_get_flag(set, flag) \
    (!!(((setfields_t *)(set))->flags & (flag)))

// Set flag `flag` active
#define bitset_set_flag(set, flag) \
    ((setfields_t *)(set))->flags |= (uint16_t)(flag)

// Clear the flag `flag` if set
#define bitset_clear_flag(set, flag) \
    ((setfields_t *)(set))->flags &= ~(uint16_t)(flag)

// Return a bitset's tag
#define bitset_get_tag(set) \
    (((setfields_t *)(set))->tag)

// Set a bitset's tag
#define bitset_set_tag(set, val) \
    ((setfields_t *)(set))->tag = (uint16_t)(val)

// Increment a bitset's tag value
#define bitset_inc_key(set) \
    (((setfields_t *)(set))->tag++)

// Decrement a bitset's tag value
#define bitset_dec_tag(set) \
    (((setfields_t *)(set))->tag--)
