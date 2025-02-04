#pragma once

#ifdef PEXT
#include <x86intrin.h>
#endif

#include "piece.h"

// Tetris bitboard implementation
// Board is represented as 10 columns of u64
// We use column-major bitboard to ultilize special instructions, such as lzcnt or pext
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
    i32 get_count();
public:
    bool is_empty();
    bool is_occupied(i8 x, i8 y);
public:
    i32 clear();
    void print();
};