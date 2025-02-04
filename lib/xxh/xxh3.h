#pragma once

#include <iostream>
#include <cstdint>
#include <stdalign.h>
#include <bit>

struct XXH128_hash_t
{
    uint64_t low64;
    uint64_t high64;
};

#define XXH_PRIME64_1  0x9E3779B185EBCA87ULL
#define XXH_PRIME64_2  0xC2B2AE3D27D4EB4FULL
#define XXH_PRIME64_3  0x165667B19E3779F9ULL
#define XXH_PRIME64_4  0x85EBCA77C2B2AE63ULL
#define XXH_PRIME64_5  0x27D4EB2F165667C5ULL

#define XXH3_MIDSIZE_STARTOFFSET 3
#define XXH3_MIDSIZE_LASTOFFSET 17

#define XXH3_SECRET_DEFAULT_SIZE 192
#define XXH3_SECRET_SIZE_MIN 136

alignas(64) static const uint8_t XXH3_K_SECRET[XXH3_SECRET_DEFAULT_SIZE] = {
    0xb8, 0xfe, 0x6c, 0x39, 0x23, 0xa4, 0x4b, 0xbe, 0x7c, 0x01, 0x81, 0x2c, 0xf7, 0x21, 0xad, 0x1c,
    0xde, 0xd4, 0x6d, 0xe9, 0x83, 0x90, 0x97, 0xdb, 0x72, 0x40, 0xa4, 0xa4, 0xb7, 0xb3, 0x67, 0x1f,
    0xcb, 0x79, 0xe6, 0x4e, 0xcc, 0xc0, 0xe5, 0x78, 0x82, 0x5a, 0xd0, 0x7d, 0xcc, 0xff, 0x72, 0x21,
    0xb8, 0x08, 0x46, 0x74, 0xf7, 0x43, 0x24, 0x8e, 0xe0, 0x35, 0x90, 0xe6, 0x81, 0x3a, 0x26, 0x4c,
    0x3c, 0x28, 0x52, 0xbb, 0x91, 0xc3, 0x00, 0xcb, 0x88, 0xd0, 0x65, 0x8b, 0x1b, 0x53, 0x2e, 0xa3,
    0x71, 0x64, 0x48, 0x97, 0xa2, 0x0d, 0xf9, 0x4e, 0x38, 0x19, 0xef, 0x46, 0xa9, 0xde, 0xac, 0xd8,
    0xa8, 0xfa, 0x76, 0x3f, 0xe3, 0x9c, 0x34, 0x3f, 0xf9, 0xdc, 0xbb, 0xc7, 0xc7, 0x0b, 0x4f, 0x1d,
    0x8a, 0x51, 0xe0, 0x4b, 0xcd, 0xb4, 0x59, 0x31, 0xc8, 0x9f, 0x7e, 0xc9, 0xd9, 0x78, 0x73, 0x64,
    0xea, 0xc5, 0xac, 0x83, 0x34, 0xd3, 0xeb, 0xc3, 0xc5, 0x81, 0xa0, 0xff, 0xfa, 0x13, 0x63, 0xeb,
    0x17, 0x0d, 0xdd, 0x51, 0xb7, 0xf0, 0xda, 0x49, 0xd3, 0x16, 0x55, 0x26, 0x29, 0xd4, 0x68, 0x9e,
    0x2b, 0x16, 0xbe, 0x58, 0x7d, 0x47, 0xa1, 0xfc, 0x8f, 0xf8, 0xb8, 0xd1, 0x7a, 0xd0, 0x31, 0xce,
    0x45, 0xcb, 0x3a, 0x8f, 0x95, 0x16, 0x04, 0x28, 0xaf, 0xd7, 0xfb, 0xca, 0xbb, 0x4b, 0x40, 0x7e,
};

static inline uint64_t xxh_read_le_64(const void* ptr)
{
    const uint8_t* q = (const uint8_t*)ptr;
    uint64_t result = 0;
    result |= q[7]; result <<= 8;
    result |= q[6]; result <<= 8;
    result |= q[5]; result <<= 8;
    result |= q[4]; result <<= 8;
    result |= q[3]; result <<= 8;
    result |= q[2]; result <<= 8;
    result |= q[1]; result <<= 8;
    result |= q[0];
    return result;
};

static inline uint32_t xxh_read_le_32(const void* ptr)
{
    const uint8_t* q = (const uint8_t*)ptr;
    uint32_t result = 0;
    result |= q[3]; result <<= 8;
    result |= q[2]; result <<= 8;
    result |= q[1]; result <<= 8;
    result |= q[0];
    return result;
};

static uint32_t xxh_swap_32(uint32_t x)
{
    return
        ((x << 24) & 0xff000000 ) |
        ((x << 8) & 0x00ff0000 ) |
        ((x >> 8) & 0x0000ff00 ) |
        ((x >> 24) & 0x000000ff );
}

static uint64_t xxh_swap_64(uint64_t x)
{
    return
        ((x << 56) & 0xff00000000000000ULL) |
        ((x << 40) & 0x00ff000000000000ULL) |
        ((x << 24) & 0x0000ff0000000000ULL) |
        ((x << 8) & 0x000000ff00000000ULL) |
        ((x >> 8) & 0x00000000ff000000ULL) |
        ((x >> 24) & 0x0000000000ff0000ULL) |
        ((x >> 40) & 0x000000000000ff00ULL) |
        ((x >> 56) & 0x00000000000000ffULL);
};

static inline const uint64_t xxh_xorshift64(uint64_t v64, int shift)
{
    return v64 ^ (v64 >> shift);
};

static uint64_t xxh3_rrmxmx(uint64_t h64, uint64_t len)
{
    h64 ^= std::rotl(h64, 49) ^ std::rotl(h64, 24);
    h64 *= 0x9FB21C651E98DF25ULL;
    h64 ^= (h64 >> 35) + len ;
    h64 *= 0x9FB21C651E98DF25ULL;
    return xxh_xorshift64(h64, 28);
}

static XXH128_hash_t xxh_mult64_to128(uint64_t lhs, uint64_t rhs)
{
    __uint128_t const product = (__uint128_t)lhs * (__uint128_t)rhs;
    XXH128_hash_t r128;
    r128.low64  = uint64_t(product);
    r128.high64 = uint64_t(product >> 64);
    return r128;
};

static uint64_t xxh3_mul128_fold64(uint64_t lhs, uint64_t rhs)
{
    XXH128_hash_t product = xxh_mult64_to128(lhs, rhs);
    return product.low64 ^ product.high64;
};

static inline uint64_t xxh3_mix16B(const uint8_t* input, const uint8_t* secret, uint64_t seed64)
{
    const uint64_t input_lo = xxh_read_le_64(input);
    const uint64_t input_hi = xxh_read_le_64(input + 8);
    return xxh3_mul128_fold64(
        input_lo ^ (xxh_read_le_64(secret) + seed64),
        input_hi ^ (xxh_read_le_64(secret + 8) - seed64)
    );
};

static uint64_t xxh3_avalanche(uint64_t h64)
{
    h64 = xxh_xorshift64(h64, 37);
    h64 *= 0x165667919E3779F9ULL;
    h64 = xxh_xorshift64(h64, 32);
    return h64;
};

static uint64_t xxh64_avalanche(uint64_t hash)
{
    hash ^= hash >> 33;
    hash *= XXH_PRIME64_2;
    hash ^= hash >> 29;
    hash *= XXH_PRIME64_3;
    hash ^= hash >> 32;
    return hash;
}

static inline uint64_t xxh3_len_9to16_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed)
{
    const uint64_t bitflip1 = (xxh_read_le_64(secret + 24) ^ xxh_read_le_64(secret + 32)) + seed;
    const uint64_t bitflip2 = (xxh_read_le_64(secret + 40) ^ xxh_read_le_64(secret + 48)) - seed;
    const uint64_t input_lo = xxh_read_le_64(input) ^ bitflip1;
    const uint64_t input_hi = xxh_read_le_64(input + len - 8) ^ bitflip2;
    const uint64_t acc = len + xxh_swap_64(input_lo) + input_hi + xxh3_mul128_fold64(input_lo, input_hi);
    return xxh3_avalanche(acc);
};

static inline uint64_t xxh3_len_4to8_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed)
{
    seed ^= (uint64_t)xxh_swap_32((uint32_t)seed) << 32;
    const uint32_t input1 = xxh_read_le_32(input);
    const uint32_t input2 = xxh_read_le_32(input + len - 4);
    const uint64_t bitflip = (xxh_read_le_64(secret + 8) ^ xxh_read_le_64(secret + 16)) - seed;
    const uint64_t input64 = input2 + (((uint64_t)input1) << 32);
    const uint64_t keyed = input64 ^ bitflip;
    return xxh3_rrmxmx(keyed, len);
};

static inline uint64_t xxh3_len_1to3_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed)
{
    const uint8_t c1 = input[0];
    const uint8_t c2 = input[len >> 1];
    const uint8_t c3 = input[len - 1];
    const uint32_t combined = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24) | ((uint32_t)c3 << 0) | ((uint32_t)len << 8);
    const uint64_t bitflip = (xxh_read_le_32(secret) ^ xxh_read_le_32(secret + 4)) + seed;
    const uint64_t keyed = (uint64_t)combined ^ bitflip;
    return xxh64_avalanche(keyed);
};

static inline uint64_t xxh3_len_0to16_64b(const uint8_t* input, size_t len, const uint8_t* secret, uint64_t seed)
{
    if (__builtin_expect(len >  8, 1)) return xxh3_len_9to16_64b(input, len, secret, seed);
    if (__builtin_expect(len >= 4, 1)) return xxh3_len_4to8_64b(input, len, secret, seed);
    if (len) return xxh3_len_1to3_64b(input, len, secret, seed);
    return xxh64_avalanche(seed ^ (xxh_read_le_64(secret + 56) ^ xxh_read_le_64(secret + 64)));
};

static inline uint64_t xxh3_len_17to128_64b(const uint8_t* input, size_t len, const uint8_t* secret, size_t secret_size, uint64_t seed)
{
    uint64_t acc = len * XXH_PRIME64_1, acc_end;

    acc += xxh3_mix16B(input + 0, secret + 0, seed);
    acc_end = xxh3_mix16B(input + len - 16, secret + 16, seed);

    if (len > 32) {
        acc += xxh3_mix16B(input + 16, secret + 32, seed);
        acc_end += xxh3_mix16B(input + len - 32, secret + 48, seed);

        if (len > 64) {
            acc += xxh3_mix16B(input + 32, secret + 64, seed);
            acc_end += xxh3_mix16B(input + len - 48, secret + 80, seed);

            if (len > 96) {
                acc += xxh3_mix16B(input + 48, secret + 96, seed);
                acc_end += xxh3_mix16B(input + len - 64, secret + 112, seed);
            }
        }
    }
    return xxh3_avalanche(acc + acc_end);
};

static uint64_t XXH3_len_129to240_64b(const uint8_t* input, size_t len, const uint8_t* secret, size_t secret_size, uint64_t seed)
{
    uint64_t acc = len * XXH_PRIME64_1;
    uint64_t acc_end;
    unsigned int const nbRounds = (unsigned int)len / 16;
    unsigned int i;
    
    for (i = 0; i < 8; i++) {
        acc += xxh3_mix16B(input + (16 * i), secret + (16 * i), seed);
    }
    
    acc_end = xxh3_mix16B(input + len - 16, secret + XXH3_SECRET_SIZE_MIN - XXH3_MIDSIZE_LASTOFFSET, seed);
    acc = xxh3_avalanche(acc);

    for (i = 8 ; i < nbRounds; i++) {
        acc_end += xxh3_mix16B(input + (16 * i), secret + (16 * (i - 8)) + XXH3_MIDSIZE_STARTOFFSET, seed);
    }

    return xxh3_avalanche(acc + acc_end);
}

static inline uint64_t xxh3_64(const void* input, size_t len)
{
    if (len <= 16) {
        return xxh3_len_0to16_64b((const uint8_t*)input, len, XXH3_K_SECRET, 0);
    }

    if (len <= 128) {
        return xxh3_len_17to128_64b((const uint8_t*)input, len, XXH3_K_SECRET, sizeof(XXH3_K_SECRET), 0);
    }

    if (len <= 240) {
        return XXH3_len_129to240_64b((const uint8_t*)input, len, XXH3_K_SECRET, sizeof(XXH3_K_SECRET), 0);
    }
    
    return 0;
};