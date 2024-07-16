#include "eval.h"

namespace Evaluation
{

void evaluate(Node::Data& node, Node::Data& parent, Piece::Data placement, std::vector<Piece::Type>& queue, Weight& w)
{
    // Terrain evaluation
    node.score.eval = 0;

    Board board = node.state.board;

    i32 heights[10] = { 0 };
    board.get_heights(heights);

    i32 max_height = *std::max_element(heights + 3, heights + 7);
    node.score.eval += std::max(max_height - 10, 0) * w.height_10;
    node.score.eval += std::max(max_height - 15, 0) * w.height_15;

    Piece::Type next = Piece::Type::NONE;
    if (node.state.next >= queue.size()) {
        if (node.state.bag.get_size() == 1) {
            for (i32 i = 0; i < 7; ++i) {
                if (node.state.bag.data[i]) {
                    next = Piece::Type(i);
                    break;
                }
            }
        }
    }
    else {
        next = queue[node.state.next];
    }
    node.score.eval += Evaluation::get_pc_next(board, heights, next) * w.pc;
    if (node.state.hold != Piece::Type::NONE) {
        node.score.eval += Evaluation::get_pc_next(board, heights, node.state.hold) * w.pc;
    }

    i32 structure_t[4] = { 0 };
    i32 donation_depth = (node.state.hold == Piece::Type::T) + (node.state.bag[static_cast<u8>(Piece::Type::T)]);
    Evaluation::get_donation(board, heights, donation_depth, structure_t);
    for (i32 i = 0; i < 4; ++i) {
        node.score.eval += structure_t[i] * w.structure[i];
    }

    max_height = *std::max_element(heights + 3, heights + 7);
    node.score.eval += max_height * w.height;

    i32 well_index = 0;
    i32 well_depth = Evaluation::get_well(board, heights, well_index);
    node.score.eval += std::clamp(well_depth, 0, 6) * w.well;

    const i32 well_x_coef[10] = { -4, -3, -2, -1, 0, 0, -1, -2, -3, -4 };
    node.score.eval += well_x_coef[well_index] * w.well_x;

    i32 bump = 0;
    i32 bump_s = 0;
    Evaluation::get_bump(heights, well_index, bump, bump_s);
    node.score.eval += bump * w.bump;
    node.score.eval += bump_s * w.bump_s;

    i32 resource = Evaluation::get_resource(board, heights[well_index]);
    node.score.eval += std::clamp(resource, 0, 36) * w.resource;

    node.score.eval += Evaluation::get_parity(board, heights[well_index]) * w.parity;

    node.score.eval += Evaluation::get_parity_vertical(board, node.state.hold, node.state.bag) * w.parity_v;

    node.score.eval += Evaluation::get_border(board) * w.border;

    node.score.eval += Evaluation::get_hole(board, heights) * w.hole;

    node.score.eval += Evaluation::get_cover(board, heights) * w.cover;

    if (node.state.b2b > 0) {
        node.score.eval += w.have_b2b;
    }

    // Accumulating evaluation
    if (node.lock.clear > 0) {
        if (node.lock.pc) {
            node.score.acml += w.pc;
        }
        else {
            if (node.lock.tspin) {
                node.score.acml += w.tspin[node.lock.clear - 1];
            }
            else {
                node.score.acml += w.clear[node.lock.clear - 1];
            }
        }
    }

    if (parent.state.b2b > node.state.b2b && !node.lock.pc) {
        node.score.acml += w.break_b2b;
    }

    if (node.lock.clear > 0 && node.state.ren > 1) {
        node.score.acml += w.ren_bonus;
    }

    if (node.lock.clear == 0 && parent.state.ren > 1) {
        node.score.acml -= (parent.state.ren - 1) * w.ren_bonus;
    }

    if (node.lock.clear > 0 && node.state.b2b > 1) {
        node.score.acml += w.b2b;
    }

    if (!node.lock.pc) {
        if (node.state.ren > 10) {
            node.score.acml += w.ren[4];
        }
        else if (node.state.ren > 8) {
            node.score.acml += w.ren[3];
        }
        else if (node.state.ren > 6) {
            node.score.acml += w.ren[2];
        }
        else if (node.state.ren > 4) {
            node.score.acml += w.ren[1];
        }
        else if (node.state.ren > 2) {
            node.score.acml += w.ren[0];
        }
    }

    if (node.lock.softdrop && !(node.lock.tspin && node.lock.clear > 0) && !node.lock.pc) {
        node.score.acml += std::max(20 - placement.y, 0) * w.waste_time;
    }

    if (placement.type != queue[parent.state.next]) {
        node.score.acml += w.waste_time;
    }

    if (placement.type == Piece::Type::T && !(node.lock.tspin && node.lock.clear > 0) && !node.lock.pc) {
        node.score.acml += w.waste_T;
    }
};

void get_bump(i32 heights[10], i32 well_index, i32& bump, i32& bump_s)
{
    i32 pre_index = 0;

    if (well_index == 0) {
        pre_index = 1;
    }

    for (i32 i = 1; i < 10; ++i) {
        if (i == well_index) {
            continue;
        }

        i32 height_different = std::abs(heights[pre_index] - heights[i]);

        bump += height_different;
        bump_s += height_different * height_different;
        pre_index = i;
    }
};

void get_donation(Board& board, i32 heights[10], i32 depth, i32 structure_t[4])
{
    for (i32 i = 0; i < depth; ++i) {
        Board copy = board;
        Piece::Data quiet_piece = Evaluation::get_structure(copy, heights);

        if (quiet_piece.type == Piece::Type::NONE) {
            break;
        }

        copy.place_piece(quiet_piece);
        i32 line_clear = copy.clear_line();
        structure_t[line_clear]++;

        if (line_clear >= 2) {
            board = copy;
            board.get_heights(heights);
        }
        else {
            break;
        }
    }
};

i32 get_hole(Board& board, i32 heights[10])
{
    i32 result = 0;

    for (i32 i = 0; i < 10; ++i) {
        result += heights[i] - std::popcount(board[i]);
    }

    return result;
};

i32 get_cover(Board& board, i32 heights[10])
{
    i32 result = 0;

    for (i32 i = 0; i < 10; ++i) {
        u64 mask = (1ULL << heights[i]) - 1;
        u64 holes = ~board[i] & mask;

        result += heights[i] - 64 + std::countl_zero(holes);
    }

    return result;
};

i32 get_well(Board& board, i32 heights[10], i32& well_index)
{
    well_index = 0;

    for (int i = 1; i < 10; ++i) {
        if (heights[i] < heights[well_index]) {
            well_index = i;
        }
    }

    u64 mask = ~0b0;

    for (int i = 0; i < 10; ++i) {
        if (i == well_index) continue;
        mask = mask & board[i];
    }

    mask = mask >> heights[well_index];

    return std::countr_one(mask);
};

i32 get_transition_row(Board& board, i32 heights[10])
{
    i32 result = 0;

    for (i32 i = 0; i < 9; ++i) {
        u64 xor_column = board[i] ^ board[i + 1];
        result += std::popcount(xor_column);
    }

    result += 64 - std::popcount(board[0]);
    result += 64 - std::popcount(board[9]);

    return result;
};

i32 get_parity(Board& board, i32 min_height)
{
    i32 color[2] = { 0 };

    for (i32 i = 0; i < 10; ++i) {
        u64 col = board[i] >> min_height;
        i32 col_count = std::popcount(col);
        i32 col_color_count = std::popcount(col & (0xAAAAAAAAAAAAAAAA >> i));
        color[i & 1] += col_color_count;
        color[(i + 1) & 1] += col_count - col_color_count;
    }

    return std::abs(color[0] - color[1]);
};

i32 get_parity_vertical(Board& board, Piece::Type hold, Bag bag)
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

    change_possible += bag[static_cast<i32>(Piece::Type::J)] * 2;
    change_possible += bag[static_cast<i32>(Piece::Type::L)] * 2;
    change_possible += bag[static_cast<i32>(Piece::Type::T)] * 2;
    change_possible += bag[static_cast<i32>(Piece::Type::I)] * 4;

    change_possible += (hold == Piece::Type::J) * 2;
    change_possible += (hold == Piece::Type::L) * 2;
    change_possible += (hold == Piece::Type::T) * 2;
    change_possible += (hold == Piece::Type::I) * 4;

    return std::max(parity - change_possible, 0);
};

i32 get_resource(Board& board, i32 min_height)
{
    i32 result = 0;

    for (i32 i = 0; i < 10; ++i) {
        u64 col = board[i] >> min_height;

        result += std::popcount(col);
    }

    return result;
};

i32 get_border(Board& board)
{
    i32 result = 0;

    auto copy = board;

    while (!copy.is_perfect())
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

Piece::Data get_structure(Board& board, i32 heights[10])
{
    for (i32 x = 0; x < 8; ++x) {
        if (heights[x + 0] > heights[x + 1] && heights[x + 0] + 1 < heights[x + 2]) {
            if (((board[x + 0] >> (heights[x + 0] - 1)) & 0b111) == 0b001 &&
                ((board[x + 1] >> (heights[x + 0] - 1)) & 0b111) == 0b000 &&
                ((board[x + 2] >> (heights[x + 0] - 1)) & 0b111) == 0b101) {
                return Piece::Data
                (
                    i8(x + 1),
                    i8(heights[x + 0]),
                    Piece::Type::T,
                    Piece::Rotation::DOWN
                );
            }
        }
        if (heights[x + 2] > heights[x + 1] && heights[x + 2] + 1 < heights[x + 0]) {
            if (((board[x + 0] >> (heights[x + 2] - 1)) & 0b111) == 0b101 &&
                ((board[x + 1] >> (heights[x + 2] - 1)) & 0b111) == 0b000 &&
                ((board[x + 2] >> (heights[x + 2] - 1)) & 0b111) == 0b001) {
                return Piece::Data
                (
                    i8(x + 1),
                    i8(heights[x + 2]),
                    Piece::Type::T,
                    Piece::Rotation::DOWN
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
                return Piece::Data
                (
                    i8(x + 2),
                    i8(heights[x + 1] - 2),
                    Piece::Type::T,
                    Piece::Rotation::LEFT
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
                return Piece::Data
                (
                    i8(x),
                    i8(heights[x + 1] - 2),
                    Piece::Type::T,
                    Piece::Rotation::RIGHT
                );
            }
        }
    }

    return Piece::Data();
};

bool get_pc_next(Board& board, i32 heights[10], Piece::Type next)
{
    if (next == Piece::Type::NONE) {
        return false;
    }
    
    i32 count = board.get_count();
    if (count != 6 && count != 16 && count != 26 && count != 36) {
        return false;
    }

    if (next == Piece::Type::I) {
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
    else if (next == Piece::Type::O) {
        if (count == 16) {
            for (i32 i = 0; i < 9; ++i) {
                if (board[i] == 0 && board[i + 1] == 0) {
                    return true;
                }
            }
        }
    }
    else if (next == Piece::Type::T) {
        if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 1 && board[i + 1] == 0 && board[i + 2] == 1) {
                    return true;
                }
            }
        }
    }
    else if (next == Piece::Type::S) {
        if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 2 && board[i + 1] == 0 && board[i + 2] == 1) {
                    return true;
                }
            }
        }
    }
    else if (next == Piece::Type::Z) {
        if (count == 16) {
            for (i32 i = 0; i < 8; ++i) {
                if (board[i] == 1 && board[i + 1] == 0 && board[i + 2] == 2) {
                    return true;
                }
            }
        }
    }
    else if (next == Piece::Type::L) {
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
    else if (next == Piece::Type::J) {
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