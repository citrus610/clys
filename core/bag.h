#pragma once

#include "piece.h"

// Tetris bag for the 7-bag rule
class Bag
{
public:
    u8 data;
public:
    Bag();
public:
    bool get(const piece::Type& piece);
public:
    void update(const piece::Type& next);
    void deupdate(const piece::Type& next);
public:
    i32 get_size();
public:
    void print();
};