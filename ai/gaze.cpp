#include "gaze.h"

namespace Gaze
{

bool can_offset(Player self, Player enemy, Piece::Data placement)
{
    if (self.pc) {
        return false;
    }

    auto state = State();
    state.board = self.board;
    auto lock = state.lock(placement);

    if (lock.clear > 0 || lock.softdrop) {
        return false;
    }

    i32 heights[10];
    self.board.get_heights(heights);

    i32 height_mid = *std::max_element(heights + 3, heights + 7);

    i32 board_count = self.board.get_count();
    bool board_even = ((board_count & 1) == 0);

    i32 well_index = 0;
    i32 well_depth = Evaluation::get_well(self.board, heights, well_index);

    // Check if our height is too high
    if (heights[well_index] >= 4) {
        return false;
    }

    if (height_mid >= 8) {
        return false;
    }

    // Check if our board is even and it's low enough for a possible pc in the future
    bool board_pc_possible = board_even && heights[well_index] == 0 && board_count < 40;

    // Check available counters
    // i32 counter_tetris = Gaze::get_available_tetris(self, 2);
    // i32 counter_tspin = Gaze::get_available_tspin(self, 2);
    // i32 counter = counter_tetris + counter_tspin;
    i32 counter = Gaze::get_available_attacks(self, 1);

    // Check enemy's attack
    bool enemy_pc = Gaze::get_available_pc(enemy);
    // i32 enemy_tetris = Gaze::get_available_tetris(enemy, 2);
    // i32 enemy_tspin = Gaze::get_available_tspin(enemy, 2);
    // i32 enemy_attack = enemy_tetris + enemy_tspin;
    i32 enemy_attack = Gaze::get_available_attacks(enemy, 2);

    if (enemy_pc && height_mid > 4) {
        return false;
    }

    if (counter == 0 || enemy_attack == 0) {
        return false;
    }

    if (board_pc_possible && enemy_attack < 2) {
        return false;
    }

    return (counter >= enemy_attack) || (counter > 0 && enemy_pc);
};

i32 get_available_tetris(Player self, i32 depth)
{
    i32 heights[10];
    self.board.get_heights(heights);

    i32 well_index = 0;
    i32 well_depth = Evaluation::get_well(self.board, heights, well_index);

    i32 near_I = 0;

    near_I += self.current == Piece::Type::I;
    near_I += self.hold == Piece::Type::I;

    for (i32 i = 0; i < depth; ++i) {
        if (self.queue[i] == Piece::Type::I) {
            near_I += 1;
        }
    }

    return std::min(well_depth / 4, near_I);
};

i32 get_available_tspin(Player self, i32 depth)
{
    i32 heights[10];
    self.board.get_heights(heights);

    i32 near_T = 0;

    near_T += self.current == Piece::Type::T;
    near_T += self.hold == Piece::Type::T;

    for (i32 i = 0; i < depth; ++i) {
        if (self.queue[i] == Piece::Type::T) {
            near_T += 1;
        }
    }

    i32 structure_t[4] = { 0 };
    Evaluation::get_donation(self.board, heights, 3, structure_t);

    i32 structure_count = 0;
    for (i32 i = 1; i < 4; ++i) {
        structure_count += structure_t[i];
    }

    return std::min(structure_count, near_T);
};

i32 get_available_attacks(Player self, i32 depth)
{
    i32 attacks = 0;

    i32 heights[10];
    self.board.get_heights(heights);

    i32 near_I = 0;

    near_I += self.current == Piece::Type::I;
    near_I += self.hold == Piece::Type::I;

    for (i32 i = 0; i < depth; ++i) {
        if (self.queue[i] == Piece::Type::I) {
            near_I += 1;
        }
    }

    i32 near_T = 0;

    near_T += self.current == Piece::Type::T;
    near_T += self.hold == Piece::Type::T;

    for (i32 i = 0; i < depth; ++i) {
        if (self.queue[i] == Piece::Type::T) {
            near_T += 1;
        }
    }

    while (true)
    {
        if (near_I <= 0 && near_T <= 0) {
            break;
        }

        i32 well_index = 0;
        i32 well_depth = Evaluation::get_well(self.board, heights, well_index);

        bool tetris = near_I > 0 && well_depth > 3;

        if (tetris) {
            attacks += 4;

            near_I -= 1;

            auto drop_I = Piece::Data(
                well_index,
                20,
                Piece::Type::I,
                Piece::Rotation::RIGHT
            );

            drop_I.move_drop(self.board);

            self.board.place_piece(drop_I);

            self.board.clear_line();
            self.board.get_heights(heights);
        }

        i32 structure_t[4] = { 0 };
        Evaluation::get_donation(self.board, heights, near_T, structure_t);

        i32 structure_count = 0;
        for (i32 i = 1; i < 4; ++i) {
            structure_count += structure_t[i];
        }

        bool tspin = near_T > 0 && structure_count > 0;

        if (tspin) {
            for (i32 i = 1; i < 4; ++i) {
                attacks += structure_t[i] * i * 2;
            }

            near_T -= structure_count;
        }

        if (!tetris && !tspin) {
            break;
        }
    }

    return attacks;
};

bool get_available_pc(Player self)
{
    auto queue = self.queue;

    if (queue.size() > 2) {
        queue.erase(queue.begin() + 2, queue.end());
    }

    queue.insert(queue.begin(), self.current);

    State state = State();
    state.board = self.board;
    state.hold = self.hold;

    auto bresult = Beam::search(queue, state, Lock());

    return bresult.pc;
};

};