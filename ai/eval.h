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
    i32 mid = 0;
    i32 mid_10 = 0;
    i32 mid_15 = 0;
    i32 bump = 0;
    i32 volume = 0;
    i32 parity = 0;
    i32 parity_v = 0;
    i32 border = 0;
    i32 hole = 0;
    i32 cover = 0;
    i32 well = 0;
    i32 map = 0;
    std::array<i32, 3> tsd_slot = { 0 };
    std::array<i32, 4> tst_slot = { 0 };
    i32 b2b_bonus = 0;
    i32 ren_bonus = 0;
    i32 pcable = 0;

    std::array<i32, 3> tspin = { 0 };
    std::array<i32, 4> clear = { 0 };
    i32 pc = 0;
    i32 b2b = 0;
    i32 b2b_cont = 0;
    std::array<i32, 5> ren = { 0 };
    i32 waste_time = 0;
    i32 waste_T = 0;
};

void evaluate(node::Data& node, node::Data& parent, move::Placement placement, const std::vector<piece::Type>& queue, const Weight& w);

i32 get_bump(i32 heights[10], i32 well_x);

i32 get_hole(Board& board, i32 heights[10]);

i32 get_cover(Board& board, i32 heights[10]);

i32 get_well(Board& board, i32 heights[10], i32& well_x);

i32 get_parity(Board& board);

i32 get_parity_vertical(Board& board, piece::Type hold, Bag& bag);

i32 get_border(Board& board);

i32 get_volume(Board& board, i32 heights[10], i32 well_x);

move::Placement get_structure(Board& board, i32 heights[10]);

void get_donation(Board& board, i32 heights[10], i32 depth, i32 tsd_slot[3], i32 tst_slot[4]);

bool get_pc_next(Board& board, i32 heights[10], piece::Type next);

i32 get_pc_able(Board& board, i32 heights[10], i32 next, const std::vector<piece::Type>& queue, const piece::Type& hold);

bool get_pc_split(Board& board, i32 height);

bool get_pc_fillable(Board& board, i32 height);

bool get_pc_parity(Board& board, i32 need, i32 next, const std::vector<piece::Type>& queue, const piece::Type& hold);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    mid,
    mid_10,
    mid_15,
    bump,
    volume,
    parity,
    parity_v,
    border,
    hole,
    cover,
    well,
    map,
    tsd_slot,
    tst_slot,
    b2b_bonus,
    ren_bonus,
    pcable,
    tspin,
    clear,
    pc,
    b2b,
    b2b_cont,
    ren,
    waste_time,
    waste_T
)

};