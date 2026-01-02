#pragma once

#include "piece.h"

class Board
{
public:
    u64 data[10] {};
public:
    u64& operator [] (usize index);
    bool operator == (Board& other);
    bool operator != (Board& other);
public:
    std::array<i32, 10> get_heights();
    i32 get_count();
public:
    bool is_empty();
    bool is_occupied(i8 x, i8 y);
public:
    i32 clear_lines();
    void print();
};