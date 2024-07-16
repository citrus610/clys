#pragma once

#include "../ai/ai.h"

namespace Tuner
{

struct Score
{
    i32 attack = 0;
    i32 pc = 0;
    i32 ren = 0;
    i32 tspin = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Score, attack, pc, ren, tspin)

void place_garbage(Board& board, i32 garbage)
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

// 5 lines = 320 = 0.6
// 1 line = 80 = 0.15
// 3 lines = 64 = 0.12
// 2 lines = 48 = 0.1
// 6 lines = 10 = 0.015
// 10 lines = 8 = 0.015
std::vector<i32> generate_garbage()
{
    std::vector<i32> result;

    i32 map_w[] = { 600, 150, 120, 100, 15, 15 };
    i32 map_l[] = { 5, 1, 3, 2, 6, 10 };

    for (int i = 0; i < 100; ++i) {
        i32 rng = rand() % 1000;

        i32 acml = 0;

        for (i32 k = 0; k < 6; ++k) {
            acml += map_w[k];

            if (acml > rng) {
                result.push_back(map_l[k]);
                break;
            }
        }
    }

    return result;
};

static Score get_score(std::vector<Piece::Type> q, std::vector<i32> g, Evaluation::Weight w)
{
    Score result = Score();

    std::vector<Piece::Type> init_q = { q[0], q[1], q[2], q[3], q[4] };
    q.erase(q.begin(), q.begin() + 5);

    AI::Engine ai = AI::Engine();
    ai.init(init_q, Lock(), State(), w);

    i32 delay = 0;
    i32 frame = 0;

    const i32 clear_delay[] = { 0, 35, 40, 40, 45 };

    const i32 garbage_amount = 5;
    const i32 garbage_interval = 128;
    const i32 garbage_delay = 30;
    i32 garbage_frame = 0;
    i32 garbage_delay_frame = -1;
    i32 garbage = 0;

    i32 garbage_index = 0;

    i32 spike = 0;

    while (frame < 3600)
    {
        ai.search();

        std::this_thread::sleep_for(std::chrono::microseconds(delay * 16667));

        auto r = ai.request(0);

        if (!r.has_value()) {
            break;
        }

        bool first_hold = ai.root_state.hold == Piece::Type::NONE && r.value().placement.type != ai.queue.front();

        std::vector<Piece::Type> next = { q[0] };
        q.erase(q.begin());

        if (first_hold) {
            next.push_back(q[0]);
            q.erase(q.begin());
        }

        auto state_copy = ai.root_state;
        auto lock = state_copy.advance(r.value().placement, ai.queue);

        if (lock.clear == 0) {
            spike = 0;
        }
        else {
            spike += Beam::get_attack(state_copy, lock);
        }

        // if (lock.clear == 4) result.tetris += 1;
        if (lock.tspin && lock.clear > 0) result.tspin += 1;
        if (lock.pc) result.pc += 1;
        result.ren = std::max(result.ren, state_copy.ren);

        if (!ai.advance(r.value().placement, next)) {
            break;
        }

        delay = 7;

        delay += (i32(r.value().placement.x) - 4) * 2;

        if (lock.softdrop) {
            delay += std::max(0, 19 - i32(r.value().placement.y)) * 2;
        }

        if (lock.clear > 0) {
            if (lock.pc) {
                delay += 1;
            }
            else {
                delay += clear_delay[lock.clear];
            }
        }

        frame += delay;

        i32 attack = Beam::get_attack(state_copy, lock);

        result.attack += attack;

        // printf("\rframe: %d/3600        ", frame);

        garbage_frame += delay;

        if (garbage_frame >= garbage_interval) {
            garbage_frame = 0;
            garbage = g[garbage_index % g.size()];
            garbage_delay_frame = 0;
            garbage_index += 1;
            continue;
        }

        if (garbage_delay_frame > -1) {
            garbage_delay_frame += delay;
            garbage -= attack;

            if (garbage_delay_frame >= garbage_delay) {
                if (garbage > 0) {
                    Tuner::place_garbage(ai.root_state.board, garbage);
                    delay = 2;
                }

                garbage_delay_frame = -1;
                garbage = 0;
            }
        }
    }

    ai.clear();

    printf("\n");

    return result;
};

static bool operator < (const Score& a, const Score& b)
{
    if (std::abs(a.attack - b.attack) > 2) {
        return a.attack < b.attack;
    }

    if (a.pc != b.pc) {
        return a.pc < b.pc;
    }

    if (std::abs(a.ren - b.ren) > 2) {
        return a.ren < b.ren;
    }

    if (a.tspin != b.tspin) {
        return a.tspin < b.tspin;
    }

    return a.attack < b.attack;
};

};