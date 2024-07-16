#pragma once

#include "../ai/ai.h"

namespace Battle
{

static std::vector<Piece::Type> generate_queue()
{
    std::vector<Piece::Type> result;

    std::vector<Piece::Type> full = {
        Piece::Type::I,
        Piece::Type::J,
        Piece::Type::L,
        Piece::Type::O,
        Piece::Type::S,
        Piece::Type::T,
        Piece::Type::Z
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

static void place_garbage(Board& board, i32 garbage)
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
    AI::Engine ai;
    std::optional<AI::Plan> plan;
    Evaluation::Weight w;
public:
    Sync sync;
    Delay delay;
public:
    State state;
    std::vector<Piece::Type> queue;
    std::vector<Garbage> garbages;
    i32 queue_index = 0;
    bool running;
public:
    i32 attack_total;
    i32 pc;
    i32 tspin;
public:
    Player();
public:
    void init(Evaluation::Weight w);
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
    void init(Evaluation::Weight w1, Evaluation::Weight w2);
    void update();
    void end();
};

};