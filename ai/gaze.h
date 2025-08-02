#pragma once

#include "eval.h"

namespace gaze
{

struct Player
{
    Board board;
    std::vector<piece::Type> queue;
    piece::Type current;
    piece::Type hold;
};

bool is_tankable(Player self, move::Placement placement, i32 incomming);

};