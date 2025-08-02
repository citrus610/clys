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
    std::array<i32, 10> height_map;
    std::array<i32, 10> well_map;
    i32 bump;
    i32 transition;
    i32 hole_a;
    i32 hole_b;
    i32 cover;
    std::array<i32, 4> tslot;
    i32 bonus_b2b;
    i32 bonus_ren;
    i32 scale;

    i32 pc;
    i32 pc_next;
    std::array<i32, 3> tspin;
    std::array<i32, 4> clear;
    i32 b2b;
    std::array<i32, 5> ren;
    i32 feed;
    i32 order;
    i32 delay;
    i32 waste_T;
    i32 waste_I;
};

void evaluate(node::Data& node, node::Data& parent, move::Placement placement, const std::vector<piece::Type>& queue, const Weight& w);

std::pair<i32, i32> get_well(Board& board, i32 heights[10]);

i32 get_bump(i32 heights[10], i32 well_x);

i32 get_transition(Board& board, i32 well_x);

std::pair<i32, i32> get_hole(Board& board, i32 heights[10], i32 well_x);

i32 get_cover(Board& board, i32 heights[10]);

move::Placement get_structure(Board& board, i32 heights[10]);

void get_donation(Board& board, i32 heights[10], i32 depth, i32 tslot[4]);

bool get_pc_next(Board& board, i32 heights[10], piece::Type next);

i32 get_pc_able(Board& board, i32 heights[10]);

bool get_pc_split(Board& board, i32 height);

bool get_pc_fillable(Board& board, i32 height);

bool get_pc_parity(Board& board, i32 need, i32 next, const std::vector<piece::Type>& queue, const piece::Type& hold);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    height,
    well,
    height_map,
    well_map,
    bump,
    transition,
    hole_a,
    hole_b,
    cover,
    tslot,
    bonus_b2b,
    bonus_ren,
    scale,
    pc,
    pc_next,
    tspin,
    clear,
    b2b,
    ren,
    feed,
    order,
    delay,
    waste_T,
    waste_I
)

};