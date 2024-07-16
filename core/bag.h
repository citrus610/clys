#pragma once

#include "def.h"
#include "piece.h"

class Bag
{
public:
    bool data[7];
public:
    Bag();
    Bag(bool init[7]);
public:
    bool& operator [] (i32 index);
public:
    void update(Piece::Type next);
    void deupdate(Piece::Type next);
public:
    i32 get_size();
public:
    void print();
};