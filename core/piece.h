#pragma once

#include "arrayvec.h"
#include "def.h"

namespace piece
{

enum class Type : u8 {
    I,
    J,
    L,
    O,
    S,
    T,
    Z,
    NONE
};

enum class Rotation : u8 {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NONE
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

constexpr char get_char(Type type)
{
    switch (type)
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