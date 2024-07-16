#pragma once

#include "piece.h"

class Board
{
public:
    u64 cols[10] = { 0 };
public:
    u64& operator [] (i32 index);
    bool operator == (Board& other);
    bool operator != (Board& other);
public:
    void get_heights(i32 heights[10]);
    u64 get_clear_mask();
    i32 get_drop_distance(Piece::Data& piece);
    i32 get_count();
public:
    bool is_perfect();
    bool is_occupied(i8 x, i8 y);
    bool is_colliding(i8 x, i8 y, Piece::Type type, Piece::Rotation r);
    bool is_colliding(Piece::Data& piece);
    bool is_above_stack(Piece::Data& piece);
    bool is_tspin(Piece::Data& piece);
public:
    i32 clear_line();
    void place_piece(Piece::Data& piece);
public:
    void print();
};