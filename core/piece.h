#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <cmath>
#include <chrono>
#include <vector>
#include <random>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <bitset>
#include <bit>
#include <thread>
#include <mutex>
#include <atomic>
#include <optional>
#include <functional>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

namespace piece
{

enum class Type : u8
{
    I,
    J,
    L,
    O,
    S,
    T,
    Z,
    NONE
};

enum class Rotation : u8
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

constexpr i8 LUT[7][4][4][2] = {
    {
        {{ -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }},
        {{ 0, 1 }, { 0, 0 }, { 0, -1 }, { 0, -2 }},
        {{ 1, 0 }, { 0, 0 }, { -1, 0 }, { -2, 0 }},
        {{ 0, -1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }}
    },
    {
        {{ -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }},
        {{ 0, 1 }, { 0, 0 }, { 0, -1 }, { 1, 1 }},
        {{ 1, 0 }, { 0, 0 }, { -1, 0 }, { 1, -1 }},
        {{ 0, -1 }, { 0, 0 }, { 0, 1 }, { -1, -1 }}
    },
    {
        {{ -1, 0 }, { 0, 0 }, { 1, 0 }, { 1, 1 }},
        {{ 0, 1 }, { 0, 0 }, { 0, -1 }, { 1, -1 }},
        {{ 1, 0 }, { 0, 0 }, { -1, 0 }, { -1, -1 }},
        {{ 0, -1 }, { 0, 0 }, { 0, 1 }, { -1, 1 }}
    },
    {
        {{ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }}, 
        {{ 0, 0 }, { 0, -1 }, { 1, 0 }, { 1, -1 }},
        {{ 0, 0 }, { -1, 0 }, { 0, -1 }, { -1, -1 }},
        {{ 0, 0 }, { 0, 1 }, { -1, 0 }, { -1, 1 }}
    },
    {
        {{ -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }},
        {{ 0, 1 }, { 0, 0 }, { 1, 0 }, { 1, -1 }},
        {{ 1, 0 }, { 0, 0 }, { 0, -1 }, { -1, -1 }},
        {{ 0, -1 }, { 0, 0 }, { -1, 0 }, { -1, 1 }}
    },
    {
        {{ -1, 0 }, { 0, 0 }, { 1, 0 }, { 0, 1 }},
        {{ 0, 1 }, { 0, 0 }, { 0, -1 }, { 1, 0 }},
        {{ 1, 0 }, { 0, 0 }, { -1, 0 }, { 0, -1 }},
        {{ 0, -1 }, { 0, 0 }, { 0, 1 }, { -1, 0 }}
    },
    {
        {{ -1, 1 }, { 0, 1 }, { 0, 0 }, { 1, 0 }},
        {{ 1, 1 }, { 1, 0 }, { 0, 0 }, { 0, -1 }},
        {{ 1, -1 }, { 0, -1 }, { 0, 0 }, { -1, 0 }},
        {{ -1, -1 }, { -1, 0 }, { 0, 0 }, { 0, 1 }}
    }
};

constexpr i8 get_offset_x(const Type& type, const Rotation& r, i32 index)
{
    return LUT[static_cast<u8>(type)][static_cast<u8>(r)][index][0];
};

constexpr i8 get_offset_x(i32 type, i32 r, i32 index)
{
    return LUT[type][r][index][0];
};

constexpr i8 get_offset_y(const Type& type, const Rotation& r, i32 index)
{
    return LUT[static_cast<u8>(type)][static_cast<u8>(r)][index][1];
};

constexpr i8 get_offset_y(i32 type, i32 r, i32 index)
{
    return LUT[type][r][index][1];
};

constexpr char to_char(Type piece)
{
    switch (piece)
    {
    case Type::I:
        return 'I';
    case Type::J:
        return 'J';
    case Type::L:
        return 'L';
    case Type::O:
        return 'O';
    case Type::S:
        return 'S';
    case Type::T:
        return 'T';
    case Type::Z:
        return 'Z';
    default:
        return ' ';
    }

    return ' ';
};

};