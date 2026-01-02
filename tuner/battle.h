#pragma once

#include "../ai/ai.h"
#include <random>

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

    auto rng = std::default_random_engine{ (unsigned int)rand() };

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
    i32 max_spike = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Result,
    win,
    attack,
    max_spike
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
    i32 attack;
    i32 max_spike;
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

inline std::tuple<Result, Result, i32> do_battle(eval::Weight w_p1, eval::Weight w_p2, std::vector<piece::Type> q_p1, std::vector<piece::Type> q_p2, i32 iter)
{
    battle::Game game;
    game.init(w_p1, w_p2, q_p1, q_p2);

    i32 frame = 0;

    battle::Result r1 = battle::Result();
    battle::Result r2 = battle::Result();

    while (true)
    {
        game.update();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        frame += 1;
        printf("\rframe: %d/%d                   ", frame, iter);

        if (game.players[0].is_dead() || game.players[1].is_dead() || frame >= iter) {
            if (!game.players[0].ren.empty()) {
                i32 spike = std::accumulate(game.players[0].ren.begin(), game.players[0].ren.end(), 0);

                game.players[0].max_spike = std::max(game.players[0].max_spike, spike);
            }

            if (!game.players[1].ren.empty()) {
                i32 spike = std::accumulate(game.players[1].ren.begin(), game.players[1].ren.end(), 0);
                
                game.players[1].max_spike = std::max(game.players[1].max_spike, spike);
            }

            r1 = battle::Result {
                .win = game.players[1].is_dead(),
                .attack = game.players[0].attack,
                .max_spike = game.players[0].max_spike
            };

            r2 = battle::Result {
                .win = game.players[0].is_dead(),
                .attack = game.players[1].attack,
                .max_spike = game.players[1].max_spike
            };

            break;
        }
    };

    game.end();

    return { r1, r2, std::max(1, frame) };
};

};