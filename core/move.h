#pragma once

#include "board.h"

namespace move
{

constexpr usize MAX = 128;

constexpr i8 SRS_LUT[2][4][5][2] = {
    {
        {{ 0, 0 }, { -1, 0 }, { 2, 0 }, { -1, 0 }, { 2, 0 }},
        {{ -1, 0 }, { 0, 0 }, { 0, 0 }, { 0, 1 }, { 0, -2 }},
        {{ -1, 1 }, { 1, 1 }, { -2, 1 }, { 1, 0 }, { -2, 0 }},
        {{ 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, -1 }, { 0, 2 }},
    },
    {
        {{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }},
        {{ 0, 0 }, { 1, 0 }, { 1, -1 }, { 0, 2 }, { 1, 2 }},
        {{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }},
        {{ 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, 2 }, { -1, 2 }},
    }
};

constexpr i8 LOCK_HEIGHT_LUT[7][4] = {
    { 19, 21, 19, 20 },
    { 19, 20, 20, 20 },
    { 19, 20, 20, 20 },
    { 19, 20, 20, 19 },
    { 19, 20, 20, 20 },
    { 19, 20, 20, 20 },
    { 19, 20, 20, 20 }
};

class Placement
{
public:
    i8 x;
    i8 y;
    piece::Rotation r;
    piece::Type type;
public:
    Placement() : x(0), y(0), r(piece::Rotation::NONE), type(piece::Type::NONE) {};
    Placement(i8 x, i8 y, piece::Rotation r, piece::Type type) : x(x), y(y), r(r), type(type) {};
public:
    bool operator == (const Placement& other);
    bool operator != (const Placement& other);
public:
    bool is_colliding(Board& board);
    bool is_tspin(Board& board);
    bool is_above_stack(Board& board);
public:
    Placement get_normalize();
public:
    void place(Board& board);
    void normalize();
    void print();
};

class Map
{
public:
    u64 data[4][10] = { 0 };
public:
    bool get(i8 x, i8 y, piece::Rotation r);
    void set(i8 x, i8 y, piece::Rotation r);
    void unset(i8 x, i8 y, piece::Rotation r);
    void clear();
};

bool move_right(Placement& piece, Map& collision);

bool move_left(Placement& piece, Map& collision);

bool move_cw(Placement& piece, Map& collision);

bool move_ccw(Placement& piece, Map& collision);

bool move_rotate(Placement& piece, piece::Rotation new_r, Map& collision);

void move_drop(Placement& piece, Map& collision);

bool is_convex(const Board& board, const Map& map_collision);

arrayvec<Placement, MAX> generate(const Board& board, piece::Type type);

void expand(const Placement& piece, Map& collision, Map& visited, Map& locked, arrayvec<Placement, MAX>& result);

constexpr i8 get_srs_x(const piece::Type& piece, const piece::Rotation& r, i32 kick)
{
    return SRS_LUT[piece != piece::Type::I][static_cast<u8>(r)][kick][0];
};

constexpr i8 get_srs_x(i32 index, const piece::Rotation& r, i32 kick)
{
    return SRS_LUT[index][static_cast<u8>(r)][kick][0];
};

constexpr i8 get_srs_y(const piece::Type& piece, const piece::Rotation& r, i32 kick)
{
    return SRS_LUT[piece != piece::Type::I][static_cast<u8>(r)][kick][1];
};

constexpr i8 get_srs_y(i32 index, const piece::Rotation& r, i32 kick)
{
    return SRS_LUT[index][static_cast<u8>(r)][kick][1];
};

constexpr i8 get_lock_height(const piece::Type& piece, const piece::Rotation& r)
{
    return LOCK_HEIGHT_LUT[static_cast<u8>(piece)][static_cast<u8>(r)];
};

inline void bench()
{
    using namespace std;

    Board board;

    auto bench = [&] (Board b) {
        const int count = 1000000;

        for (i8 t = 0; t < 7; ++t) {
            i64 time = 0;
            i32 c = 0;

            std::vector<i64> lists;
            lists.reserve(count);

            for (int i = 0; i < count; ++i) {
                auto time_start = chrono::high_resolution_clock::now();
                auto m = move::generate(b, piece::Type(t));
                auto time_stop = chrono::high_resolution_clock::now();

                auto dt = chrono::duration_cast<chrono::nanoseconds>(time_stop - time_start).count();

                c += m.size();
                time += dt;
                lists.push_back(dt);
            }

            time = time / count;
            c = c / count;

            u64 sd = 0;
            u64 max = 0;
            u64 min = UINT64_MAX;

            for (auto dt : lists) {
                sd += (dt - time) * (dt - time);
                max = std::max(max, u64(dt));
                min = std::min(min, u64(dt));
            }

            sd = sd / count;

            cout <<
                "\tpiece: " << piece::get_char(piece::Type(t)) <<
                "\ttime: " << time << " ns" <<
                "\tstdev: " << std::sqrt(sd) <<
                "\tmin: " << min << " ns" <<
                "\tmax: " << max << " ns" <<
                "\tcount: " << c << endl;
        }
    };

    board[9] = 0b00111111;
    board[8] = 0b00111111;
    board[7] = 0b00011111;
    board[6] = 0b00000111;
    board[5] = 0b00000001;
    board[4] = 0b00000000;
    board[3] = 0b00001101;
    board[2] = 0b00011111;
    board[1] = 0b00111111;
    board[0] = 0b11111111;

    cout << "BOARD TSPIN" << endl;

    bench(board);

    board[9] = 0b111111111;
    board[8] = 0b111111111;
    board[7] = 0b011111111;
    board[6] = 0b011111111;
    board[5] = 0b000111111;
    board[4] = 0b000100110;
    board[3] = 0b010000001;
    board[2] = 0b011110111;
    board[1] = 0b011111111;
    board[0] = 0b011111111;

    cout << "BOARD DT CANNON" << endl;

    bench(board);
    
    board[9] = 0b000011111111;
    board[8] = 0b000011000000;
    board[7] = 0b110011001100;
    board[6] = 0b110011001100;
    board[5] = 0b110011001100;
    board[4] = 0b110011001100;
    board[3] = 0b110011001100;
    board[2] = 0b110000001100;
    board[1] = 0b110000001100;
    board[0] = 0b111111111100;

    cout << "BOARD TERRIBLE" << endl;

    bench(board);

    board[9] = 0b00000;
    board[8] = 0b00011;
    board[7] = 0b00011;
    board[6] = 0b00011;
    board[5] = 0b00111;
    board[4] = 0b01111;
    board[3] = 0b01111;
    board[2] = 0b11111;
    board[1] = 0b11111;
    board[0] = 0b11111;

    cout << "BOARD CONVEX" << endl;
    
    bench(board);
};

};