#pragma once

#include "../ai/ai.h"

namespace battle
{

inline std::vector<piece::Type> generate_queue()
{
    std::vector<piece::Type> result;

    std::vector<piece::Type> full = {
        piece::Type::I,
        piece::Type::J,
        piece::Type::L,
        piece::Type::O,
        piece::Type::S,
        piece::Type::T,
        piece::Type::Z
    };

    auto rng = std::default_random_engine { (unsigned int)rand() };

    for (i32 i = 0; i < 256; i++) {
        auto full_cp = full;

        std::shuffle(full_cp.begin(), full_cp.end(), rng);

        for (auto p : full_cp) {
            result.push_back(p);
        }
    }

    return result;
};

inline void place_garbage(Board& board, i32 garbage)
{
    i32 index = rand() % 10;

    for (int i = 0; i < garbage; ++i) {
        for (int k = 0; k < 10; ++k) {
            board[k] = (board[k] << 1) | 1ULL;
        }

        board[index] = board[index] & (~1ULL);

        if ((rand() % 100) >= 70) {
            index = rand() % 10;
        }
    }
};

struct Garbage
{
    i32 frame = 0;
    i32 count = 0;
};

struct Delay
{
    i32 start = 15;
    i32 spawn = 7;
    i32 das = 2;
    i32 softdrop = 2;
    i32 clear[4] = { 35, 40, 40, 45 };
    i32 pc = 1;
    i32 garbage = 30;
};

struct Result
{
    bool win = false;
    i32 attack = 0;
    i32 pc = 0;
    i32 tspin = 0;
    i32 tetris = 0;
    i32 ren_6 = 0;
    i32 ren_8 = 0;
    i32 ren_10 = 0;
    i32 ren_12 = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Result,
    attack,
    pc,
    tspin,
    tetris,
    ren_6,
    ren_8,
    ren_10,
    ren_12
)

class Sync
{
public:
    i32 counter;
public:
    Sync();
public:
    void clear();
    void wait(i32 frame);
    bool is_waiting();
};

class Player
{
public:
    ai::Engine ai;
    std::optional<ai::Plan> plan;
    eval::Weight w;
public:
    Sync sync;
    Delay delay;
public:
    State state;
    std::vector<piece::Type> queue;
    std::vector<Garbage> garbages;
    i32 queue_index = 0;
    bool running;
public:
    i32 attack_total;
    i32 pc;
    i32 tspin;
    i32 tetris;
    std::vector<i32> ren;
public:
    Player();
public:
    void init(eval::Weight w, std::vector<piece::Type> q);
    void update(Player* enemy);
    void end();
public:
    bool is_dead();
};

class Game
{
public:
    Player players[2];
public:
    void init(eval::Weight w1, eval::Weight w2, std::vector<piece::Type> q1, std::vector<piece::Type> q2);
    void update();
    void end();
};

};