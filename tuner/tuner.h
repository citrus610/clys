#pragma once

#include <fstream>
#include "battle.h"

namespace tuner
{

struct SaveData {
    eval::Weight w;
    battle::Result result;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, w, result)

void save(std::string id, SaveData s)
{
    std::string name = std::string("data/") + id + std::string(".json");
    std::ofstream o(name);
    json js;
    to_json(js, s);
    o << std::setw(4) << js << std::endl;
    o.close();
};

void load(std::string id, SaveData& s)
{
    std::string name = std::string("data/") + id + std::string(".json");
    std::ifstream file;
    file.open(name);
    json js;
    file >> js;
    file.close();
    from_json(js, s);
};

inline void print_w(eval::Weight w)
{
    #define PRW(p) printf("%s: %d\n", #p, w.p);

    PRW(well);
    PRW(b2b);
    PRW(ren_bonus);
    PRW(ren[0]);
    PRW(ren[1]);
    PRW(ren[2]);
    PRW(tspin[0]);
    PRW(tspin[1]);

    PRW(clear[0]);
    PRW(clear[1]);
    PRW(clear[2]);
    PRW(waste_T);
};

eval::Weight constrain(eval::Weight w)
{
    #define CONSTRAIN_POSITIVE(p) w.p = std::max(0, w.p);
    #define CONSTRAIN_NEGATIVE(p) w.p = std::min(0, w.p);
    #define CONSTRAIN_CLAMP(p, m1, m2) w.p = std::clamp(w.p, m1, m2);

    CONSTRAIN_NEGATIVE(mid);
    CONSTRAIN_NEGATIVE(mid_10);
    CONSTRAIN_NEGATIVE(mid_15);
    CONSTRAIN_CLAMP(bump, -100, -25);
    CONSTRAIN_CLAMP(volume, 1, 50);
    CONSTRAIN_NEGATIVE(parity);
    CONSTRAIN_CLAMP(border, -100, -25);
    CONSTRAIN_NEGATIVE(hole);
    CONSTRAIN_NEGATIVE(cover);
    CONSTRAIN_POSITIVE(well);
    CONSTRAIN_POSITIVE(map);
    CONSTRAIN_POSITIVE(tsd_slot[0]);
    CONSTRAIN_POSITIVE(tsd_slot[1]);
    CONSTRAIN_POSITIVE(tsd_slot[2]);
    CONSTRAIN_POSITIVE(tst_slot[0]);
    CONSTRAIN_POSITIVE(tst_slot[1]);
    CONSTRAIN_POSITIVE(tst_slot[2]);
    CONSTRAIN_POSITIVE(tst_slot[3]);
    CONSTRAIN_POSITIVE(b2b_bonus);
    CONSTRAIN_POSITIVE(ren_bonus);
    CONSTRAIN_CLAMP(pcable, -500, -25);

    CONSTRAIN_POSITIVE(tspin[0]);
    CONSTRAIN_POSITIVE(tspin[1]);
    CONSTRAIN_POSITIVE(tspin[2]);
    CONSTRAIN_NEGATIVE(clear[0]);
    CONSTRAIN_NEGATIVE(clear[1]);
    CONSTRAIN_NEGATIVE(clear[2]);
    CONSTRAIN_POSITIVE(clear[3]);
    CONSTRAIN_POSITIVE(pc);
    CONSTRAIN_POSITIVE(b2b);
    CONSTRAIN_POSITIVE(ren[0]);
    CONSTRAIN_POSITIVE(ren[1]);
    CONSTRAIN_POSITIVE(ren[2]);
    CONSTRAIN_POSITIVE(ren[3]);
    CONSTRAIN_POSITIVE(ren[4]);
    CONSTRAIN_NEGATIVE(waste_time);
    CONSTRAIN_NEGATIVE(waste_T);

    return w;
};

std::pair<eval::Weight, eval::Weight> randomize(eval::Weight w, i32 id)
{
    auto w1 = w;
    auto w2 = w;
    auto w_pre = w;

    std::pair<i32*, i32> param[] = {
        // { &w.mid, 1 },
        // { &w.mid_10, 1 },
        // { &w.mid_15, 1 },
        // { &w.bump, 1 },
        { &w.volume, 1 },
        // { &w.parity, 1 },
        // { &w.border, 1 },
        { &w.hole, 15 },
        { &w.cover, 1 },
        { &w.well, 1 },
        // { &w.map, 5 },
        // { &w.tsd_slot[0], 15 },
        // { &w.tsd_slot[1], 15 },
        // { &w.tsd_slot[2], 15 },
        // { &w.tst_slot[0], 15 },
        // { &w.tst_slot[1], 15 },
        // { &w.tst_slot[2], 15 },
        // { &w.tst_slot[3], 15 },
        // { &w.b2b_bonus, 15 },
        { &w.ren_bonus, 15 },
        // { &w.pcable, 2 },
        // { &w.tspin[0], 10 },
        // { &w.tspin[1], 10 },
        // { &w.tspin[2], 10 },
        { &w.clear[0], 10 },
        { &w.clear[1], 10 },
        { &w.clear[2], 10 },
        { &w.clear[3], 10 },
        // { &w.pc, 1 },
        // { &w.b2b, 1 },
        { &w.ren[0], 15 },
        { &w.ren[1], 15 },
        { &w.ren[2], 20 },
        // { &w.waste_time, 1 },
        { &w.waste_T, 10 }
    };

    i32 param_delta[_countof(param)] = { 0 };

    for (size_t i = 0; i < _countof(param); ++i) {
        i32 delta = param[i].second;

        i32 sign = (rand() % 2) * 2 - 1;
        i32 value = delta;

        param_delta[i] = value * sign;
    }

    for (size_t i = 0; i < _countof(param); ++i) {
        *param[i].first += param_delta[i];
    }

    w1 = constrain(w);

    w = w_pre;

    for (size_t i = 0; i < _countof(param); ++i) {
        *param[i].first -= param_delta[i];
    }

    w2 = constrain(w);

    return { w1, w2 };
};

i32 match(eval::Weight w1, eval::Weight w2, battle::Result& r1, battle::Result& r2)
{
    auto cmp_result = [&] (const battle::Result& a, const battle::Result& b) -> bool {
        if (a.win != b.win) {
            return a.win < b.win;
        }

        if (a.attack == b.attack) {
            return a.pc < b.pc;
        }

        return a.attack < b.attack;
    };

    auto battle_bot = [] (eval::Weight w_p1, eval::Weight w_p2, std::vector<piece::Type> q_p1, std::vector<piece::Type> q_p2, i32 iter) -> std::tuple<battle::Result, battle::Result, i32> {
        battle::Game game;
        game.init(w_p1, w_p2, q_p1, q_p2);

        i32 frame = 0;

        battle::Result r1 = battle::Result();
        battle::Result r2 = battle::Result();

        while (true)
        {
            game.update();

            std::this_thread::sleep_for(std::chrono::milliseconds(12));

            frame += 1;
            printf("\rframe: %d/%d        ", frame, iter);

            if (game.players[0].is_dead() || game.players[1].is_dead() || frame >= iter) {
                r1 = battle::Result {
                    .win = game.players[1].is_dead(),
                    .attack = game.players[0].attack_total,
                    .pc = game.players[0].pc,
                    .tspin = game.players[0].tspin,
                    .tetris = game.players[0].tetris,
                    .ren_6 = 0,
                    .ren_8 = 0,
                    .ren_10 = 0,
                    .ren_12 = 0
                };

                r2 = battle::Result {
                    .win = game.players[0].is_dead(),
                    .attack = game.players[1].attack_total,
                    .pc = game.players[1].pc,
                    .tspin = game.players[1].tspin,
                    .tetris = game.players[1].tetris,
                    .ren_6 = 0,
                    .ren_8 = 0,
                    .ren_10 = 0,
                    .ren_12 = 0
                };

                for (auto ren : game.players[0].ren) {
                    if (ren >= 12) {
                        r1.ren_12 += 1;
                    }
                    else if (ren >= 10) {
                        r1.ren_10 += 1;
                    }
                    else if (ren >= 8) {
                        r1.ren_8 += 1;
                    }
                    else if (ren >= 6) {
                        r1.ren_6 += 1;
                    }
                }

                for (auto ren : game.players[1].ren) {
                    if (ren >= 12) {
                        r2.ren_12 += 1;
                    }
                    else if (ren >= 10) {
                        r2.ren_10 += 1;
                    }
                    else if (ren >= 8) {
                        r2.ren_8 += 1;
                    }
                    else if (ren >= 6) {
                        r2.ren_6 += 1;
                    }
                }

                break;
            }
        };

        game.end();

        return { r1, r2, std::max(1, frame) };
    };

    auto q1 = battle::generate_queue();
    auto q2 = battle::generate_queue();

    printf("Matching w+ vs w-...\n");

    const auto iter = 10000;

    auto r = battle_bot(w1, w2, q1, q2, iter);

    r1 = std::get<0>(r);
    r2 = std::get<1>(r);

    r1.attack = r1.attack * 36000 / std::get<2>(r);
    r2.attack = r2.attack * 36000 / std::get<2>(r);

    printf("\n");

    system("cls");

    printf("w+: win - %d | apm - %d | pc - %d | tspin - %d | tetris - %d | ren12 - %d | ren10 - %d | ren8 - %d | ren6 - %d\n", r1.win, r1.attack / 10, r1.pc, r1.tspin, r1.tetris, r1.ren_12, r1.ren_10, r1.ren_8, r1.ren_6);
    printf("w-: win - %d | apm - %d | pc - %d | tspin - %d | tetris - %d | ren12 - %d | ren10 - %d | ren8 - %d | ren6 - %d\n", r2.win, r2.attack / 10, r2.pc, r2.tspin, r2.tetris, r2.ren_12, r2.ren_10, r2.ren_8, r2.ren_6);

    if (cmp_result(r2, r1)) {
        return 1;
    }

    if (cmp_result(r1, r2)) {
        return -1;
    }

    return 0;
};

void move_toward(eval::Weight& w, eval::Weight target, i32 id)
{
    #define MOVE_TOWARD(p, r) w.p += i32(std::round(double(target.p - w.p) * r));

    MOVE_TOWARD(mid, 0.1);
    MOVE_TOWARD(mid_10, 0.1);
    MOVE_TOWARD(mid_15, 0.1);
    MOVE_TOWARD(bump, 0.5);
    MOVE_TOWARD(volume, 0.5);
    MOVE_TOWARD(parity, 0.5);
    MOVE_TOWARD(border, 0.5);
    MOVE_TOWARD(hole, 0.1);
    MOVE_TOWARD(cover, 0.5);
    MOVE_TOWARD(well, 0.5);
    MOVE_TOWARD(map, 0.1);
    MOVE_TOWARD(tsd_slot[0], 0.1);
    MOVE_TOWARD(tsd_slot[1], 0.1);
    MOVE_TOWARD(tsd_slot[2], 0.1);
    MOVE_TOWARD(tst_slot[0], 0.1);
    MOVE_TOWARD(tst_slot[1], 0.1);
    MOVE_TOWARD(tst_slot[2], 0.1);
    MOVE_TOWARD(tst_slot[3], 0.1);
    MOVE_TOWARD(b2b_bonus, 0.1);
    MOVE_TOWARD(ren_bonus, 0.1);
    MOVE_TOWARD(pcable, 0.5);
    MOVE_TOWARD(tspin[0], 0.1);
    MOVE_TOWARD(tspin[1], 0.1);
    MOVE_TOWARD(tspin[2], 0.1);
    MOVE_TOWARD(clear[0], 0.1);
    MOVE_TOWARD(clear[1], 0.1);
    MOVE_TOWARD(clear[2], 0.1);
    MOVE_TOWARD(clear[3], 0.1);
    MOVE_TOWARD(pc, 0.1);
    MOVE_TOWARD(b2b, 0.1);
    MOVE_TOWARD(ren[0], 0.1);
    MOVE_TOWARD(ren[1], 0.1);
    MOVE_TOWARD(ren[2], 0.1);
    MOVE_TOWARD(ren[3], 0.1);
    MOVE_TOWARD(ren[4], 0.1);
    MOVE_TOWARD(waste_time, 0.5);
    MOVE_TOWARD(waste_T, 0.1);
};

inline void run(eval::Weight w, i32 id_init = 0)
{
    system("cls");

    i32 id = id_init;

    while (true)
    {
        auto randw = tuner::randomize(w, id);

        auto w1 = randw.first;
        auto w2 = randw.second;

        battle::Result r1, r2, score;

        i32 m = tuner::match(w1, w2, r1, r2);

        if (m == 1) {
            tuner::move_toward(w, w1, id);
            score = r1;
        }
        else if (m == -1) {
            tuner::move_toward(w, w2, id);
            score = r2;
        }
        else {
            continue;
        }

        score.attack = (r1.attack + r2.attack) / 2;

        auto save_data = SaveData{
            .w = w,
            .result = score
        };

        tuner::save(std::to_string(id), save_data);

        printf("id: %d\n\n", id);

        id += 1;
    }
};

};