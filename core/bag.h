#pragma once

#include "piece.h"

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
    void revert(const piece::Type& last);
public:
    i32 get_count();
public:
    void print();
};