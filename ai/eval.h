#pragma once

#include <fstream>
#include <iomanip>
#include "../lib/nlohmann/json.hpp"
using json = nlohmann::json;

#include "node.h"

namespace Evaluation
{

struct Weight
{
    i32 height = 0;
    i32 height_10 = 0;
    i32 height_15 = 0;
    i32 bump = 0;
    i32 bump_s = 0;
    i32 resource = 0;
    i32 parity = 0;
    i32 parity_v = 0;
    i32 border = 0;
    i32 hole = 0;
    i32 cover = 0;
    i32 well = 0;
    i32 well_x = 0;
    i32 structure[4] = { 0 };
    i32 have_b2b = 0;

    i32 tspin[3] = { 0 };
    i32 clear[4] = { 0 };
    i32 pc = 0;
    i32 break_b2b = 0;
    i32 b2b = 0;
    i32 ren[5] = { 0 };
    i32 ren_bonus = 0;
    i32 waste_time = 0;
    i32 waste_T = 0;
};

constexpr Weight DEFAULT = Weight
{
    .height = -50,
    .height_10 = -100,
    .height_15 = -250,
    .bump = -10,
    .bump_s = -27,
    .resource = 17,
    .parity = -61,
    .parity_v = -20,
    .border = -33,
    .hole = -409,
    .cover = -4,
    .well = 40,
    .well_x = 50,
    .structure = { 108, 210, 234, 500 },
    .have_b2b = 250,

    .tspin = { 50, 400, 750 },
    .clear = { -412, -319, -219, 300 },
    .pc = 2000,
    .break_b2b = -89,
    .b2b = 100,
    .ren = { 152, 480, 1000, 2000, 4000 },
    .ren_bonus = 200,
    .waste_time = -25,
    .waste_T = -130
};

constexpr i32 REN_LUT[] = { 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5 };

constexpr u32 CLEAR_DELAY_LUT[] = { 0, 35, 40, 40, 45 };

constexpr i32 ren_sum(i32 ren)
{
    i32 result = 0;
    for (i32 i = 3; i <= ren; ++i) {
        result += REN_LUT[std::min(i, 12)];
    }
    return result;
};

void evaluate(Node::Data& node, Node::Data& parent, Piece::Data placement, std::vector<Piece::Type>& queue, Weight& w);

void get_bump(i32 heights[10], i32 well_index, i32& bump, i32& bump_s);

void get_donation(Board& board, i32 heights[10], i32 depth, i32 structure_t[4]);

i32 get_hole(Board& board, i32 heights[10]);

i32 get_cover(Board& board, i32 heights[10]);

i32 get_well(Board& board, i32 heights[10], i32& well_index);

i32 get_parity(Board& board, i32 min_height);

i32 get_parity_vertical(Board& board, Piece::Type hold, Bag bag);

i32 get_border(Board& board);

i32 get_resource(Board& board, i32 min_height);

Piece::Data get_structure(Board& board, i32 heights[10]);

bool get_pc_next(Board& board, i32 heights[10], Piece::Type next);

u64 get_bitboard_u64(Board& board);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    height,
    height_10,
    height_15,
    bump,
    bump_s,
    resource,
    parity,
    parity_v,
    border,
    hole,
    cover,
    well,
    well_x,
    structure[0],
    structure[1],
    structure[2],
    structure[3],
    have_b2b,
    tspin[0],
    tspin[1],
    tspin[2],
    clear[0],
    clear[1],
    clear[2],
    clear[3],
    pc,
    break_b2b,
    b2b,
    ren[0],
    ren[1],
    ren[2],
    ren[3],
    ren[4],
    ren_bonus,
    waste_time,
    waste_T
)

};