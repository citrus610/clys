#include "eval.h"

namespace eval
{

void evaluate(node::Data& node, node::Data& parent, move::Placement placement, const std::vector<piece::Type>& queue, const Weight& w)
{
    node.score.eval = 0;

    auto board = node.state.board;
    auto heights = board.get_heights();

    // Pc next
    auto next = piece::Type::NONE;

    if (node.state.next >= queue.size()) {
        if (node.state.bag.get_count() == 1) {
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

    if (eval::get_pc_next(board, heights, next) || eval::get_pc_next(board, heights, node.state.hold)) {
        node.score.eval += w.pc_next;
    }

    // T slot
    i32 donations = 0;

    auto tslot = eval::get_donation(board, heights, 3, donations);

    for (i32 i = 0; i < 4; ++i) {
        node.score.eval += tslot[i] * w.tslot[i];
    }

    // Height
    node.score.eval += *std::max_element(heights.begin(), heights.end()) * w.height;

    // Well
    auto [well, well_x] = eval::get_well(board, heights);

    node.score.eval += std::min(well, 4) * w.well;

    // Bump
    node.score.eval += eval::get_bump(heights, well_x) * w.bump;

    // Center
    node.score.eval += eval::get_center(well_x) * w.center;

    // Hole
    auto [hole_a, hole_b] = eval::get_hole(board, heights, well_x);

    hole_a -= tslot[0] + tslot[1] + tslot[2] + tslot[3] - donations;

    node.score.eval += hole_a * w.hole_a;
    node.score.eval += hole_b * w.hole_b;

    // Cover
    node.score.eval += eval::get_cover(board, heights) * w.cover;

    // Bonus
    if (node.state.b2b > 0) {
        node.score.eval += w.bonus_b2b;
    }

    if (node.state.ren > 1) {
        node.score.eval += (node.state.ren - 1) * w.bonus_ren;
    }

    // Clear
    bool is_pc = node.state.board.is_empty();

    if (node.lock.clear > 0) {
        if (is_pc) {
            node.score.reward += w.pc;
        }
        else if (node.lock.tspin) {
            node.score.reward += w.tspin[node.lock.clear - 1];
        }
        else {
            node.score.reward += w.clear[node.lock.clear - 1];
        }
    }

    // B2b
    if (node.lock.clear > 0 && node.state.b2b > 1) {
        node.score.reward += w.b2b;
    }

    // Ren
    if (!is_pc) {
        if (node.state.ren > 10) {
            node.score.reward += w.ren[4];
        }
        else if (node.state.ren > 8) {
            node.score.reward += w.ren[3];
        }
        else if (node.state.ren > 6) {
            node.score.reward += w.ren[2];
        }
        else if (node.state.ren > 4) {
            node.score.reward += w.ren[1];
        }
        else if (node.state.ren > 2) {
            node.score.reward += w.ren[0];
        }
    }

    // Feed
    if (parent.state.ren == 1 && node.lock.clear == 0 && !is_pc) {
        node.score.reward += w.feed;
    }

    // Check clear action type
    bool is_bad_parent = parent.lock.clear > 0 && !(parent.lock.tspin || parent.lock.clear == 4);
    bool is_good_child = node.lock.clear > 0 && (node.lock.tspin || node.lock.clear == 4 || is_pc);

    // Order bad to good
    if (is_bad_parent && is_good_child && node.state.ren < 6) {
        node.score.reward += w.order_a;
    }

    // Order tetris to tspin
    bool is_tetris_parent = parent.lock.clear == 4;
    bool is_tspin_child = node.lock.clear > 0 && node.lock.tspin;

    if (is_tetris_parent && is_tspin_child) {
        node.score.reward += w.order_b;
    }

    // Delay
    if (node.lock.softdrop && !(node.lock.tspin && node.lock.clear > 0) && !is_pc) {
        node.score.reward += std::max(20 - placement.y, 0) * w.delay;
    }

    if (placement.type != queue[parent.state.next]) {
        node.score.reward += w.delay;
    }

    if (node.lock.softdrop && is_pc) {
        node.score.reward += w.delay;
    }

    // Waste T
    if (placement.type == piece::Type::T && !(node.lock.tspin && node.lock.clear > 0) && !is_pc) {
        node.score.reward += w.waste_T;
    }

    // Waste I
    if (placement.type == piece::Type::I && node.lock.clear < 4 && !is_pc) {
        node.score.reward += w.waste_I;
    }
};

std::pair<i32, i32> get_well(Board& board, std::array<i32, 10>& heights)
{
    i32 x = 0;

    for (int i = 1; i < 10; ++i) {
        if (heights[i] < heights[x]) {
            x = i;
        }
    }

    u64 mask = ~0b0;

    for (int i = 0; i < 10; ++i) {
        if (i == x) {
            continue;
        }

        mask = mask & board[i];
    }

    mask = mask >> heights[x];

    return { std::countr_one(mask), x };
};

i32 get_bump(std::array<i32, 10>& heights, i32 well_x)
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

        i32 value = std::abs(heights[left] - heights[i]);

        bump += value * value;
        left = i;
    }

    return bump;
};

i32 get_center(i32 well_x)
{
    if (well_x < 5) {
        return 4 - well_x;
    }
    
    return well_x - 5;
};

std::pair<i32, i32> get_hole(Board& board, std::array<i32, 10>& heights, i32 well_x)
{
    const i32 height_min = heights[well_x];

    i32 hole = 0;

    for (i32 i = 0; i < 10; ++i) {
        hole += heights[i] - height_min - std::popcount(board[i] >> height_min);
    }

    return { hole, height_min };
};

i32 get_cover(Board& board, std::array<i32, 10>& heights)
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

move::Placement get_structure(Board& board, std::array<i32, 10>& heights)
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

std::array<i32, 4> get_donation(Board& board, std::array<i32, 10>& heights, i32 depth, i32& count)
{
    std::array<i32, 4> tslot = { 0 };
    count = 0;

    for (i32 i = 0; i < depth; ++i) {
        auto copy = board;
        auto quiet = eval::get_structure(copy, heights);

        if (quiet.type == piece::Type::NONE) {
            break;
        }

        quiet.place(copy);

        i32 clear = copy.clear_lines();

        tslot[clear] += 1;

        if (clear >= 2) {
            board = copy;
            heights = board.get_heights();
            count += 1;
        }
        else {
            break;
        }
    }

    return tslot;
};

bool get_pc_next(Board& board, std::array<i32, 10>& heights, piece::Type next)
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

};