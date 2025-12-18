#include "gaze.h"

namespace gaze
{

bool is_tankable(Player self, move::Placement placement, i32 incomming)
{
    // No incomming attack
    if (incomming < 1) {
        return false;
    }

    if (self.queue.empty()) {
        return false;
    }

    // Non-line-clear action
    auto state = State();
    state.board = self.board;
    auto lock = state.lock(placement);

    if (lock.clear > 0 || lock.softdrop) {
        return false;
    }

    // Checks if pc able
    i32 heights[10];
    state.board.get_heights(heights);

    i32 pc_able = eval::get_pc_able(state.board, heights);

    if (pc_able < 6 && pc_able > 0) {
        return false;
    }

    // If the incomming attack is too big
    i32 height_mid = *std::max_element(heights + 3, heights + 7);

    if (height_mid + incomming >= 14) {
        return false;
    }

    // Checks if next action is tspin or tetris
    // Tspin
    auto tspin = eval::get_structure(state.board, heights);
    
    if (tspin.type == piece::Type::T) {
        auto board_copy = state.board;
        tspin.place(board_copy);
        auto clear = board_copy.clear();

        if (clear > 1 && (self.hold == piece::Type::T || self.queue.front() == piece::Type::T)) {
            return true;
        }
    }

    // Tetris
    auto [well, well_x] = eval::get_well(state.board, heights);

    if (well >= 4 && (self.hold == piece::Type::I || self.queue.front() == piece::Type::I)) {
        auto tetris = move::Placement(well_x, heights[well_x] + 1, piece::Rotation::LEFT, piece::Type::I);
        auto board_copy = state.board;
        tetris.place(board_copy);
        board_copy.clear();

        i32 new_heights[10];
        board_copy.get_heights(new_heights);

        auto new_tspin = eval::get_structure(board_copy, new_heights);

        bool destroy_tspin = (tspin.type == piece::Type::T) && (new_tspin.type != piece::Type::T);

        if (!destroy_tspin) {
            return true;
        }
    }

    return false;
};

};