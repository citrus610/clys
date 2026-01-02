#pragma once

#include <fstream>
#include <iomanip>
#include "../lib/nlohmann/json.hpp"
using json = nlohmann::json;

#include "node.h"

namespace eval
{

struct Weight
{
    i32 height;
    i32 well;
    i32 bump;
    i32 center;
    i32 hole_a;
    i32 hole_b;
    i32 cover;
    std::array<i32, 4> tslot;
    i32 bonus_b2b;
    i32 bonus_ren;
    i32 pc_next;

    std::array<i32, 3> tspin;
    std::array<i32, 4> clear;
    std::array<i32, 5> ren;
    i32 b2b;
    i32 pc;
    i32 feed;
    i32 order_a;
    i32 order_b;
    i32 delay;
    i32 waste_T;
    i32 waste_I;
};

void evaluate(node::Data& node, node::Data& parent, move::Placement placement, const std::vector<piece::Type>& queue, const Weight& w);

std::pair<i32, i32> get_well(Board& board, std::array<i32, 10>& heights);

i32 get_bump(std::array<i32, 10>& heights, i32 well_x);

i32 get_center(i32 well_x);

std::pair<i32, i32> get_hole(Board& board, std::array<i32, 10>& heights, i32 well_x);

i32 get_cover(Board& board, std::array<i32, 10>& heights);

move::Placement get_structure(Board& board, std::array<i32, 10>& heights);

std::array<i32, 4> get_donation(Board& board, std::array<i32, 10>& heights, i32 depth, i32& count);

bool get_pc_next(Board& board, std::array<i32, 10>& heights, piece::Type next);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    height,
    well,
    bump,
    center,
    hole_a,
    hole_b,
    cover,
    tslot,
    bonus_b2b,
    bonus_ren,
    pc_next,
    tspin,
    clear,
    ren,
    b2b,
    pc,
    feed,
    order_a,
    order_b,
    delay,
    waste_T,
    waste_I
)

};