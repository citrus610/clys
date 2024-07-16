#pragma once

#include "def.h"

class Board;

namespace Piece
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

constexpr i8 LUT[7][4][4][2] =
{
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

constexpr i8 SRS_LUT[2][4][5][2] =
{
    {
        {{ 0, 0 }, { -1, 0 }, { 2, 0 }, { -1, 0 }, { 2, 0 }},
        {{ -1, 0 }, { 0, 0 }, { 0, 0 }, { 0, 1 }, { 0, -2 }},
        {{ -1, 1 }, { 1, 1 }, { -2, 1 }, { 1, 0 }, { -2, 0 }},
        {{ 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, -1 }, { 0, 2 }},
    },
    {
        {{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }},
        {{ 0, 0 }, { 1, 0 }, { 1, -1 }, { 0, 2 }, { 1, 2 }},
        {{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }},
        {{ 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, 2 }, { -1, 2 }},
    }
};

class Data
{
public:
    i8 x;
    i8 y;
    Type type;
    Rotation r;
public:
    Data() : x(0), y(0), type(Piece::Type::NONE), r(Piece::Rotation::UP) {};
    Data(i8 x, i8 y, Piece::Type type, Piece::Rotation r) : x(x), y(y), type(type), r(r) {};
public:
    bool move_right(Board& board);
    bool move_left(Board& board);
    bool move_cw(Board& board);
    bool move_ccw(Board& board);
    bool move_rotate(Board& board, Rotation new_r);
    bool move_down(Board& board);
    void move_drop(Board& board);
public:
    bool operator == (Data& other);
    bool operator == (const Data& other);
public:
    void normalize();
    Data get_normalize();
};

constexpr i8 get_offset_x(Type type, Rotation r, i32 index)
{
    return LUT[static_cast<u8>(type)][static_cast<u8>(r)][index][0];
};

constexpr i8 get_offset_y(Type type, Rotation r, i32 index)
{
    return LUT[static_cast<u8>(type)][static_cast<u8>(r)][index][1];
};

constexpr i8 get_offset_srs_x(i32 srs_index, Rotation r, i32 index)
{
    return SRS_LUT[srs_index][static_cast<u8>(r)][index][0];
};

constexpr i8 get_offset_srs_y(i32 srs_index, Rotation r, i32 index)
{
    return SRS_LUT[srs_index][static_cast<u8>(r)][index][1];
};

constexpr char to_char(Type piece)
{
    switch (piece)
    {
    case Type::I:
        return 'I';
        break;
    case Type::J:
        return 'J';
        break;
    case Type::L:
        return 'L';
        break;
    case Type::O:
        return 'O';
        break;
    case Type::S:
        return 'S';
        break;
    case Type::T:
        return 'T';
        break;
    case Type::Z:
        return 'Z';
        break;
    }
    return ' ';
};

}