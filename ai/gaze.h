#pragma once

#include "beam.h"

namespace Gaze
{

struct Player
{
    Board board;
    std::vector<Piece::Type> queue;
    Piece::Type current;
    Piece::Type hold;
    bool pc;
};

bool can_offset(Player self, Player enemy, Piece::Data placement);

i32 get_available_tetris(Player self, i32 depth);

i32 get_available_tspin(Player self, i32 depth);

i32 get_available_attacks(Player self, i32 depth);

bool get_available_pc(Player self);

};