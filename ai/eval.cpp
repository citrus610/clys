#include "eval.h"

namespace eval
{

void evaluate(node::Data& node, node::Data& parent, move::Placement placement, const std::vector<piece::Type>& queue, const Weight& w)
{
    // Terrain evaluation
    node.score.eval = 0;

    Board board = node.state.board;

    i32 heights[10] = { 0 };
    board.get_heights(heights);

    // Height in the middle
    i32 mid = *std::max_element(heights + 3, heights + 7);
    node.score.eval += std::max(mid - 10, 0) * w.mid_10;
    node.score.eval += std::max(mid - 15, 0) * w.mid_15;

    // Pc next
    auto next = piece::Type::NONE;

    if (node.state.next >= queue.size()) {
        if (node.state.bag.get_size() == 1) {
            for (i32 i = 0; i < 7; ++i) {
                if (node.state.bag.get(piece::Type(i))) {
                    next = piece::Type(i);
                    break;
                }
            }
        }
    }
    else {
        next = queue[node.state.next];
    }

    node.score.eval += (eval::get_pc_next(board, heights, next) || eval::get_pc_next(board, heights, node.state.hold)) * w.pc;

    // Pc-able
    i32 pcable = eval::get_pc_able(board, heights);
    node.score.eval += pcable * w.pcable;

    // Structure for T spin
    i32 tsd_slot[3] = { 0 };
    i32 tst_slot[4] = { 0 };
    i32 donation_depth = std::max(i32(node.state.hold == piece::Type::T) + i32(node.state.bag.get(piece::Type::T)), 1);

    eval::get_donation(board, heights, donation_depth, tsd_slot, tst_slot);

    for (i32 i = 0; i < 3; ++i) {
        node.score.eval += tsd_slot[i] * w.tsd_slot[i];
    }

    for (i32 i = 0; i < 4; ++i) {
        node.score.eval += tst_slot[i] * w.tst_slot[i];
    }

    // Middle height
    mid = *std::max_element(heights + 3, heights + 7);
    node.score.eval += mid * w.mid;

    // Well
    i32 well_x = 0;
    i32 well = eval::get_well(board, heights, well_x);
    node.score.eval += std::clamp(well, 0, 5) * w.well;

    // Well position map
    const i32 map[10] = { -4, -3, -2, -1, 0, 0, -1, -2, -3, -4 };
    node.score.eval += map[well_x] * w.map;

    // Bumpiness
    i32 bump = eval::get_bump(heights, well_x);
    node.score.eval += bump * w.bump;

    // Volumn
    i32 volume = eval::get_volume(node.state.board, heights, well_x);
    node.score.eval += std::clamp(volume, 0, 48) * w.volume;
    node.score.eval += volume * w.volume;

    // Parity
    i32 parity = eval::get_parity(board);
    node.score.eval += parity * w.parity;

    // Parity vertical
    i32 parity_v = eval::get_parity_vertical(board, node.state.hold, node.state.bag);
    node.score.eval += parity_v * w.parity_v;

    // Border
    i32 border = eval::get_border(board);
    node.score.eval += border * w.border;

    // Hole
    i32 hole = eval::get_hole(board, heights);
    node.score.eval += hole * w.hole;

    // Cover
    i32 cover = eval::get_cover(board, heights);
    node.score.eval += cover * w.cover;

    // Bonuses
    if (node.state.b2b > 0) {
        node.score.eval += w.b2b_bonus;
    }

    if (node.state.ren > 1) {
        node.score.eval += (node.state.ren - 1) * w.ren_bonus;
    }

    // Action evaluation
    // Clear
    bool pc = node.state.board.is_empty();

    if (node.lock.clear > 0) {
        if (pc) {
            node.score.action += w.pc;
        }
        else {
            if (node.lock.tspin) {
                node.score.action += w.tspin[node.lock.clear - 1];
            }
            else {
                node.score.action += w.clear[node.lock.clear - 1];
            }
        }
    }

    // B2b
    if (node.lock.clear > 0 && node.state.b2b > 1) {
        node.score.action += w.b2b;
    }

    // Continuous b2b
    if (node.state.b2b > 1 && node.state.ren > 1) {
        node.score.action += w.b2b_cont;
    }

    // Ren
    if (node.state.ren > 10) {
        node.score.action += w.ren[4];
    }
    else if (node.state.ren > 8) {
        node.score.action += w.ren[3];
    }
    else if (node.state.ren > 6) {
        node.score.action += w.ren[2];
    }
    else if (node.state.ren > 4) {
        node.score.action += w.ren[1];
    }
    else if (node.state.ren > 2) {
        if (!pc) {
            node.score.action += w.ren[0];
        }
    }

    // Waste time
    if (node.lock.softdrop && !(node.lock.tspin && node.lock.clear > 0) && !pc) {
        node.score.action += std::max(20 - placement.y, 0) * w.waste_time;
    }

    if (placement.type != queue[parent.state.next]) {
        node.score.action += w.waste_time;
    }

    if (node.lock.softdrop && pc) {
        node.score.action += w.waste_time;
    }

    // Waste T
    if (placement.type == piece::Type::T && !(node.lock.tspin && node.lock.clear > 0) && !pc) {
        node.score.action += w.waste_T;
    }
};

// Returns the board's bumpiness
// Bumpiness is defined as the sum of squares of the height changes outside of the well
i32 get_bump(i32 heights[10], i32 well_x)
{
    i32 bump = 0;
    i32 left = 0;

    if (well_x == 0) {
        left = 1;
    }

    for (i32 i = 1; i < 10; ++i) {
        if (i == well_x) {
            continue;
        }

        bump += (heights[left] - heights[i]) * (heights[left] - heights[i]);
        left = i;
    }

    return bump;
};

// Returns number of hole in the board
i32 get_hole(Board& board, i32 heights[10])
{
    i32 hole = 0;

    for (i32 i = 0; i < 10; ++i) {
        hole += heights[i] - std::popcount(board[i]);
    }

    return hole;
};

// Evaluates how covered the holes in the board are
i32 get_cover(Board& board, i32 heights[10])
{
    i32 cover = 0;

    for (i32 i = 0; i < 10; ++i) {
        u64 mask = (1ULL << heights[i]) - 1;
        u64 holes = ~board[i] & mask;

        while (holes) {
            holes |= holes - 1;
            i32 y = std::countr_one(holes);
            holes = (holes >> y) << y;
            cover += std::min(heights[i] - y, 5);
        }
    }

    return cover;
};

// Returns the depth of the board's well
// Well position is the position of the lowest column in the board
i32 get_well(Board& board, i32 heights[10], i32& well_x)
{
    well_x = 0;

    for (int i = 1; i < 10; ++i) {
        if (heights[i] < heights[well_x]) {
            well_x = i;
        }
    }

    u64 mask = ~0b0;

    for (int i = 0; i < 10; ++i) {
        if (i == well_x) {
            continue;
        }

        mask = mask & board[i];
    }

    mask = mask >> heights[well_x];

    return std::countr_one(mask);
};

// Returns board's parity
i32 get_parity(Board& board)
{
    const u64 mask_black = 0xAAAAAAAAAAAAAAAAULL;
    const u64 mask_white = 0x5555555555555555ULL;

    i32 black = 0;
    i32 white = 0;

    for (i32 i = 0; i < 10; ++i) {
        if (i & 1) {
            black += std::popcount(board[i] & mask_black);
        }
        else {
            white += std::popcount(board[i] & mask_white);
        }
    }

    return std::abs(black - white);
};

// Return board's vertical parity
i32 get_parity_vertical(Board& board, piece::Type hold, Bag& bag)
{
    // Calculate vertical parity
    i32 parity = 0;

    for (i32 i = 0; i < 10; ++i) {
        if (i & 1) {
            parity += std::popcount(board[i]);
        }
        else {
            parity -= std::popcount(board[i]);
        }
    }

    parity = std::abs(parity);

    // Calculate the possible parity changed by pieces
    i32 change_possible = 0;

    change_possible += bag.get(piece::Type::J) * 2;
    change_possible += bag.get(piece::Type::L) * 2;
    change_possible += bag.get(piece::Type::T) * 2;
    change_possible += bag.get(piece::Type::I) * 4;

    change_possible += (hold == piece::Type::J) * 2;
    change_possible += (hold == piece::Type::L) * 2;
    change_possible += (hold == piece::Type::T) * 2;
    change_possible += (hold == piece::Type::I) * 4;

    return std::max(parity - change_possible, 0);
};

// Returns the number of horizontal holes on 2 sides of the board
i32 get_border(Board& board)
{
    i32 result = 0;

    auto copy = board;

    while (!copy.is_empty())
    {
        u32 l = 0;
        u32 r = 0;

        for (i32 i = 0; i < 5; ++i) {
            l |= (copy[i] & 1) << i;
            r |= (copy[9 - i] & 1) << i;
        }

        result += 32 - std::countl_zero(l) - std::popcount(l);
        result += 32 - std::countl_zero(r) - std::popcount(r);

        for (i32 i = 0; i < 10; ++i) {
            copy[i] = copy[i] >> 1;
        }
    }

    return result;
};

// Returns board's volume above the garbage height
i32 get_volume(Board& board, i32 heights[10], i32 well_x)
{
    i32 volume = 0;

    for (i32 i = 0; i < 10; ++i) {
        volume += std::popcount(board[i] >> heights[well_x]);
    }

    return volume;
};

// Evaluates tspin structure
move::Placement get_structure(Board& board, i32 heights[10])
{
    for (i32 x = 0; x < 8; ++x) {
        if (heights[x + 0] > heights[x + 1] && heights[x + 0] + 1 < heights[x + 2]) {
            if (((board[x + 0] >> (heights[x + 0] - 1)) & 0b111) == 0b001 &&
                ((board[x + 1] >> (heights[x + 0] - 1)) & 0b111) == 0b000 &&
                ((board[x + 2] >> (heights[x + 0] - 1)) & 0b111) == 0b101) {
                return move::Placement
                (
                    i8(x + 1),
                    i8(heights[x + 0]),
                    piece::Rotation::DOWN,
                    piece::Type::T
                );
            }
        }
        if (heights[x + 2] > heights[x + 1] && heights[x + 2] + 1 < heights[x + 0]) {
            if (((board[x + 0] >> (heights[x + 2] - 1)) & 0b111) == 0b101 &&
                ((board[x + 1] >> (heights[x + 2] - 1)) & 0b111) == 0b000 &&
                ((board[x + 2] >> (heights[x + 2] - 1)) & 0b111) == 0b001) {
                return move::Placement
                (
                    i8(x + 1),
                    i8(heights[x + 2]),
                    piece::Rotation::DOWN,
                    piece::Type::T
                );
            }
        }
        if (heights[x + 1] >= heights[x + 0] && heights[x + 1] + 1 < heights[x + 2]) {
            if (((board[x + 0] >> (heights[x + 1] - 3)) & 0b11000) == 0b00000 &&
                ((board[x + 1] >> (heights[x + 1] - 3)) & 0b11110) == 0b00100 &&
                ((board[x + 2] >> (heights[x + 1] - 3)) & 0b11111) == 0b10000 &&
                (board.is_occupied(x + 1, heights[x + 1] - 3) ||
                (!board.is_occupied(x + 1, heights[x + 1] - 3) &&
                board.is_occupied(x + 2, heights[x + 1] - 4)))) {
                return move::Placement
                (
                    i8(x + 2),
                    i8(heights[x + 1] - 2),
                    piece::Rotation::LEFT,
                    piece::Type::T
                );
            }
        }
        if (heights[x + 1] >= heights[x + 2] && heights[x + 1] + 1 < heights[x + 0]) {
            if (((board[x + 0] >> (heights[x + 1] - 3)) & 0b11111) == 0b10000 &&
                ((board[x + 1] >> (heights[x + 1] - 3)) & 0b11110) == 0b00100 &&
                ((board[x + 2] >> (heights[x + 1] - 3)) & 0b11000) == 0b00000 &&
                (board.is_occupied(x + 1, heights[x + 1] - 3) ||
                (!board.is_occupied(x + 1, heights[x + 1] - 3) &&
                board.is_occupied(x + 0, heights[x + 1] - 4)))) {
                return move::Placement
                (
                    i8(x),
                    i8(heights[x + 1] - 2),
                    piece::Rotation::RIGHT,
                    piece::Type::T
                );
            }
        }
    }

    return move::Placement();
};

// Checks for board's donations
// Updates the board's state by clearing tspin slots as if T pieces had been placed there
void get_donation(Board& board, i32 heights[10], i32 depth, i32 tsd_slot[3], i32 tst_slot[4])
{
    for (i32 i = 0; i < depth; ++i) {
        auto copy = board;
        auto quiet = eval::get_structure(copy, heights);

        if (quiet.type == piece::Type::NONE) {
            break;
        }

        quiet.place(copy);

        i32 clear = copy.clear();

        if (quiet.r == piece::Rotation::DOWN) {
            tsd_slot[clear] += 1;
        }
        else {
            tst_slot[clear] += 1;
        }

        if (clear >= 2) {
            board = copy;
            board.get_heights(heights);
        }
        else {
            break;
        }
    }
};

// Check if the board is able to do a perfect clear in the next move
bool get_pc_next(Board& board, i32 heights[10], piece::Type next)
{
    if (next == piece::Type::NONE) {
        return false;
    }
    
    i32 count = board.get_count();

    if (count != 6 && count != 16 && count != 26 && count != 36) {
        return false;
    }

    if (next == piece::Type::I) {
        if (count == 36) {
            for (i32 i = 0; i < 10; ++i) {
                if (board[i] == 0) {
                    return true;
                }
            }
        }
        else if (count == 6) {
            for (i32 i = 0; i < 7; ++i) {
                if (board[i] == 0 && board[i + 1] == 0 && board[i + 2] == 0 && board[i + 3] == 0) {
                    return true;
                }
            }
        }
    }
    else if (next == piece::Type::O) {
        if (count == 16) {
            for (i32 i = 0; i < 9; ++i) {
                if (board[i] == 0 && board[i + 1] == 0) {
                    return true;
                }
            }
        }
    }
    else if (next == piece::Type::T) {
        if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 1 && board[i + 1] == 0 && board[i + 2] == 1) {
                    return true;
                }
            }
        }
    }
    else if (next == piece::Type::S) {
        if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 2 && board[i + 1] == 0 && board[i + 2] == 1) {
                    return true;
                }
            }
        }
    }
    else if (next == piece::Type::Z) {
        if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 1 && board[i + 1] == 0 && board[i + 2] == 2) {
                    return true;
                }
            }
        }
    }
    else if (next == piece::Type::L) {
        if (count == 26) {
            for (i32 i = 0; i < 9; ++i) {
                if (board[i] == 3 && board[i + 1] == 0) {
                    return true;
                }
            }
        }
        else if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 0 && board[i + 1] == 1 && board[i + 2] == 1) {
                    return true;
                }
                else if (board[i] == 2 && board[i + 1] == 2 && board[i + 2] == 0) {
                    return true;
                }
            }
        }
    }
    else if (next == piece::Type::J) {
        if (count == 26) {
            for (i32 i = 0; i < 9; ++i) {
                if (board[i] == 0 && board[i + 1] == 3) {
                    return true;
                }
            }
        }
        else if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 1 && board[i + 1] == 1 && board[i + 2] == 0) {
                    return true;
                }
                else if (board[i] == 0 && board[i + 1] == 2 && board[i + 2] == 2) {
                    return true;
                }
            }
        }
    }

    return false;
};

i32 get_pc_able(Board& board, i32 heights[10])
{
    i32 count = board.get_count();

    if (count % 2 != 0) {
        return 0;
    }

    if (count == 0) {
        return 2;
    }

    i32 height = *std::max_element(heights, heights + 10);
    i32 empty = height * 10 - count;

    if (empty % 4 != 0) {
        height += 1;
    }

    const i32 MAX_PC_HEIGHT = 6;

    while (height <= MAX_PC_HEIGHT)
    {
        bool split = eval::get_pc_split(board, height);
        bool fillable = eval::get_pc_fillable(board, height);

        if (split && fillable) {
            break;
        }

        height += 2;
    }

    return std::clamp(height, 0, MAX_PC_HEIGHT);
};

bool get_pc_split(Board& board, i32 height)
{
    u64 mask = (1ULL << height) - 1;

    i32 empty = 0;

    for (i32 i = 0; i < 10; ++i) {
        if (board[i] == mask) {
            if (empty & 0b11) {
                return false;
            }
        }

        empty += height - std::popcount(board[i]);
    }

    return true;
};

bool get_pc_fillable(Board& board, i32 height)
{
    u64 mask = (1ULL << height) - 1;

    for (i32 i = 0; i < 10; ++i) {
        if (board[i] == mask) {
            continue;
        }

        if (board[i] == 0 && height == 4) {
            continue;
        }

        u64 hole = ~board[i] & mask;
        u64 l = (i > 0) ? board[i - 1] : mask;
        u64 r = (i < 9) ? board[i + 1] : mask;

        if ((hole & l & r) == hole) {
            return false;
        }
    }

    return true;
};

bool get_pc_parity(Board& board, i32 need, i32 next, const std::vector<piece::Type>& queue, const piece::Type& hold)
{
    // Calculate vertical parity
    i32 parity = 0;

    for (i32 i = 0; i < 10; ++i) {
        if (i & 1) {
            parity += std::popcount(board[i]);
        }
        else {
            parity -= std::popcount(board[i]);
        }
    }

    parity = std::abs(parity) / 2;

    // Calculate the maximum possible parity changed by pieces
    i32 change_max = 0;

    for (size_t i = next; i < std::min(queue.size(), size_t(need)); ++i) {
        change_max += (queue[i] == piece::Type::J);
        change_max += (queue[i] == piece::Type::L);
        change_max += (queue[i] == piece::Type::T);
        change_max += (queue[i] == piece::Type::I) * 2;
    }

    change_max += (hold == piece::Type::J);
    change_max += (hold == piece::Type::L);
    change_max += (hold == piece::Type::T);
    change_max += (hold == piece::Type::I) * 2;

    if (parity > change_max) {
        return false;
    }

    // Non L, J pieces count
    i32 non_lj = 0;

    for (size_t i = next; i < std::min(queue.size(), size_t(need)); ++i) {
        non_lj += (queue[i] != piece::Type::L) && (queue[i] != piece::Type::J);
    }

    non_lj += (hold != piece::Type::L) && (hold != piece::Type::J) && (hold != piece::Type::NONE);

    // Must change parity
    i32 must_change = need - std::min(non_lj, need);

    if ((must_change == change_max) && (((parity ^ must_change) & 1) != 0)) {
        return false;
    }

    return true;
};

};