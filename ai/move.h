#pragma once

#include "../core/piece.h"
#include "../core/board.h"

namespace Move
{

class Map
{
public:
    u64 data[4][10] = { 0 };
public:
    bool get(i8 x, i8 y, Piece::Rotation r);
    void set(i8 x, i8 y, Piece::Rotation r);
    void unset(i8 x, i8 y, Piece::Rotation r);
public:
    void clear();
};

bool move_right(Piece::Data& piece, Map& map_collision);

bool move_left(Piece::Data& piece, Map& map_collision);

bool move_cw(Piece::Data& piece, Map& map_collision);

bool move_ccw(Piece::Data& piece, Map& map_collision);

bool move_rotate(Piece::Data& piece, Piece::Rotation new_r, Map& map_collision);

void move_drop(Piece::Data& piece, Map& map_collision);

bool is_above_stack(Piece::Data& piece, Map& map_collision);

class Generator
{
public:
    std::vector<Piece::Data> queue;
    std::vector<Piece::Data> locks;
public:
    Generator();
public:
    void generate(Board& board, Piece::Type type);
    void expand(Piece::Data piece, Map& map_queue, Map& map_collision, bool fast_mode);
    void lock(Piece::Data piece, Map& map_lock, Map& map_collision);
    void clear();
};

static void print_gen(Board board, Piece::Type type) {
    using namespace std;

    Generator gen;
    gen.generate(board, type);

    for (int i = 0; i < int(gen.locks.size()); ++i) {
        auto b = board;
        auto p = gen.locks[i];

        b.place_piece(gen.locks[i]);
        b.print();
        printf("x: %d y: %d r: %d\n", p.x, p.y, static_cast<i32>(p.r));

        cin.get();
        system("cls");
    }
}

static void bench_gen()
{
    using namespace std;

    Board board;

    auto bench = [&] (Board b) {
        Generator gen;
        const int count = 1000000;
        for (int8_t t = 0; t < 7; ++t) {
            int64_t time = 0;
            int32_t c;
            for (int i = 0; i < count; ++i) {
                c = 0;
                auto time_start = chrono::high_resolution_clock::now();
                gen.generate(b, Piece::Type(t));
                c += gen.locks.size();
                auto time_stop = chrono::high_resolution_clock::now();
                time += chrono::duration_cast<chrono::nanoseconds>(time_stop - time_start).count();
            }
            time = time / count;
            cout << "    piece: " << Piece::to_char(Piece::Type(t)) << "    time: " << time << " ns" << "    count: " << c << endl;
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
};

};