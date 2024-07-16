#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <bit>
#include <chrono>
#include <iostream>

static void xxprocess(const void* data, uint32_t* state)
{
    const uint32_t* block = (const uint32_t*)data;
    state[0] = std::rotl(state[0] + block[0] * 2246822519U, 13) * 2654435761U;
    state[1] = std::rotl(state[1] + block[1] * 2246822519U, 13) * 2654435761U;
    state[2] = std::rotl(state[2] + block[2] * 2246822519U, 13) * 2654435761U;
    state[3] = std::rotl(state[3] + block[3] * 2246822519U, 13) * 2654435761U;
};

static uint32_t xxhash32(const void* input, uint32_t length, uint32_t seed)
{
    const uint32_t magic_1 = 2654435761U;
    const uint32_t magic_2 = 2246822519U;
    const uint32_t magic_3 = 3266489917U;
    const uint32_t magic_4 = 668265263U;
    const uint32_t magic_5 = 374761393U;
    const uint32_t max_buffer_size = 16;

    unsigned char buffer[max_buffer_size];
    unsigned int buffer_size = 0;
    uint32_t total_length = 0;

    uint32_t state[4] = {
        seed + magic_1 + magic_2,
        seed + magic_2,
        seed,
        seed - magic_1
    };

    total_length += length;
    const unsigned char* data = (const unsigned char*)input;

    if (length < max_buffer_size) {
        while (length-- > 0) {
            buffer[buffer_size++] = *data++;
        }
        return true;
    }

    const unsigned char* stop = data + length;
    const unsigned char* stop_block = stop - max_buffer_size;

    if (buffer_size > 0) {
        while (buffer_size < max_buffer_size) {
            buffer[buffer_size++] = *data++;
        }
        xxprocess(buffer, state);
    }

    while (data <= stop_block)
    {
        xxprocess(data, state);
        data += 16;
    }

    buffer_size = stop - data;
    for (unsigned int i = 0; i < buffer_size; ++i) {
        buffer[i] = data[i];
    }

    uint32_t result = total_length;

    if (total_length >= max_buffer_size) {
        result += std::rotl(state[0], 1) + std::rotl(state[1], 7) + std::rotl(state[2], 12) + std::rotl(state[3], 18);
    }
    else {
        result += state[2] + magic_5;
    }

    data = buffer;
    stop = data + buffer_size;

    for (; data + 4 <= stop; data += 4) {
        result = std::rotl(result + *(uint32_t*)data * magic_3, 17) * magic_4;
    }

    while (data != stop) {
        result = std::rotl(result + (*data++) * magic_5, 11) * magic_1;
    }

    result ^= result >> 15;
    result *= magic_2;
    result ^= result >> 13;
    result *= magic_3;
    result ^= result >> 16;
    return result;
};