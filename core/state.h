#pragma once

#include "piece.h"
#include "bag.h"
#include "board.h"

struct Lock
{
    u8 clear = 0;
    bool pc = false;
    bool tspin = false;
    bool softdrop = false;
};

class State
{
public:
    Board board;
    Piece::Type hold;
    Bag bag;
    i32 next;
    i32 b2b;
    i32 ren;
public:
    State() : board(Board()), hold(Piece::Type::NONE), bag(Bag()), next(0), b2b(0), ren(0) {};
    State(Board board, Piece::Type hold, Bag bag, i32 next, i32 b2b, i32 ren) : board(board), hold(hold), bag(bag), next(next), b2b(b2b), ren(ren) {};
public:
    Lock advance(Piece::Data& placement, std::vector<Piece::Type>& queue);
    Lock lock(Piece::Data& placement);
public:
    void print();
};