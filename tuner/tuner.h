#pragma once

#include <fstream>
#include "battle.h"

namespace solo::tuner
{

enum class Winner
{
    P1,
    P2,
    DRAW
};

struct SaveData {
    eval::Weight w;
    i32 winner;
    i32 loser;
    i32 avg;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, w, winner, loser, avg)

inline void save(std::string id, SaveData s)
{
    std::string name = std::string("data/") + id + std::string(".json");
    std::ofstream o(name);
    json js;
    to_json(js, s);
    o << std::setw(4) << js << std::endl;
    o.close();
};

inline void load(std::string id, SaveData& s)
{
    std::string name = std::string("data/") + id + std::string(".json");
    std::ifstream file;
    file.open(name);
    json js;
    file >> js;
    file.close();
    from_json(js, s);
};

inline eval::Weight constrain(eval::Weight w)
{
    #define CONSTRAIN_POSITIVE(p) w.p = std::max(0, w.p);
    #define CONSTRAIN_NEGATIVE(p) w.p = std::min(0, w.p);
    #define CONSTRAIN_CLAMP(p, m1, m2) w.p = std::clamp(w.p, m1, m2);

    CONSTRAIN_NEGATIVE(height);
    CONSTRAIN_POSITIVE(well);
    CONSTRAIN_NEGATIVE(bump);
    CONSTRAIN_NEGATIVE(center);
    CONSTRAIN_NEGATIVE(hole_a);
    CONSTRAIN_NEGATIVE(hole_b);
    CONSTRAIN_NEGATIVE(cover);
    CONSTRAIN_POSITIVE(tslot[0]);
    CONSTRAIN_POSITIVE(tslot[1]);
    CONSTRAIN_POSITIVE(tslot[2]);
    CONSTRAIN_POSITIVE(tslot[3]);
    CONSTRAIN_POSITIVE(bonus_b2b);
    CONSTRAIN_POSITIVE(bonus_ren);
    CONSTRAIN_POSITIVE(pc_next);

    // CONSTRAIN_POSITIVE(tspin[0]);
    CONSTRAIN_POSITIVE(tspin[1]);
    CONSTRAIN_POSITIVE(tspin[2]);
    CONSTRAIN_NEGATIVE(clear[0]);
    CONSTRAIN_NEGATIVE(clear[1]);
    CONSTRAIN_NEGATIVE(clear[2]);
    CONSTRAIN_POSITIVE(clear[3]);
    CONSTRAIN_POSITIVE(ren[0]);
    CONSTRAIN_POSITIVE(ren[1]);
    CONSTRAIN_POSITIVE(ren[2]);
    CONSTRAIN_POSITIVE(ren[3]);
    CONSTRAIN_POSITIVE(ren[4]);
    CONSTRAIN_POSITIVE(b2b);
    CONSTRAIN_POSITIVE(pc);
    CONSTRAIN_NEGATIVE(feed);
    CONSTRAIN_NEGATIVE(order_a);
    CONSTRAIN_NEGATIVE(order_b);
    CONSTRAIN_NEGATIVE(delay);
    CONSTRAIN_NEGATIVE(waste_T);
    CONSTRAIN_NEGATIVE(waste_I);

    return w;
};

inline std::pair<eval::Weight, eval::Weight> randomize(eval::Weight w, i32 id)
{
    auto w1 = w;
    auto w2 = w;
    auto w_pre = w;

    std::pair<i32*, i32> param[] = {
        { &w.height, 5 },
        { &w.well, 5 },
        { &w.bump, 2 },
        { &w.center, 5 },
        { &w.hole_a, 35 },
        { &w.hole_b, 25 },
        { &w.cover, 2 },
        { &w.tslot[0], 10 },
        { &w.tslot[1], 15 },
        { &w.tslot[2], 25 },
        { &w.tslot[3], 50 },
        { &w.bonus_b2b, 10 },
        { &w.bonus_ren, 40 },
        { &w.pc_next, 80 },
        { &w.tspin[0], 5 },
        { &w.tspin[1], 40 },
        { &w.tspin[2], 80 },
        { &w.clear[0], 25 },
        { &w.clear[1], 20 },
        { &w.clear[2], 15 },
        { &w.clear[3], 30 },
        { &w.ren[0], 15 },
        { &w.ren[1], 25 },
        { &w.ren[2], 50 },
        // { &w.ren[3], 0 },
        // { &w.ren[4], 0 },
        { &w.b2b, 10 },
        { &w.pc, 150 },
        { &w.feed, 10 },
        { &w.order_a, 10 },
        { &w.order_b, 10 },
        // { &w.delay, 0 },
        { &w.waste_T, 10 },
        { &w.waste_I, 15 }
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

inline void move_toward(eval::Weight& w, eval::Weight target, i32 id)
{
    #define MOVE_TOWARD(p, r) w.p += i32(std::round(double(target.p - w.p) * r));

    MOVE_TOWARD(height, 0.1);
    MOVE_TOWARD(well, 0.1);
    MOVE_TOWARD(bump, 0.5);
    MOVE_TOWARD(center, 0.1);
    MOVE_TOWARD(hole_a, 0.1);
    MOVE_TOWARD(hole_b, 0.1);
    MOVE_TOWARD(cover, 0.5);
    MOVE_TOWARD(tslot[0], 0.1);
    MOVE_TOWARD(tslot[1], 0.1);
    MOVE_TOWARD(tslot[2], 0.1);
    MOVE_TOWARD(tslot[3], 0.1);
    MOVE_TOWARD(bonus_b2b, 0.1);
    MOVE_TOWARD(bonus_ren, 0.1);
    MOVE_TOWARD(pc_next, 0.1);
    MOVE_TOWARD(tspin[0], 0.1);
    MOVE_TOWARD(tspin[1], 0.1);
    MOVE_TOWARD(tspin[2], 0.1);
    MOVE_TOWARD(clear[0], 0.1);
    MOVE_TOWARD(clear[1], 0.1);
    MOVE_TOWARD(clear[2], 0.1);
    MOVE_TOWARD(clear[3], 0.1);
    MOVE_TOWARD(ren[0], 0.1);
    MOVE_TOWARD(ren[1], 0.1);
    MOVE_TOWARD(ren[2], 0.1);
    MOVE_TOWARD(ren[3], 0.1);
    MOVE_TOWARD(ren[4], 0.1);
    MOVE_TOWARD(b2b, 0.1);
    MOVE_TOWARD(pc, 0.1);
    MOVE_TOWARD(feed, 0.1);
    MOVE_TOWARD(order_a, 0.1);
    MOVE_TOWARD(order_b, 0.1);
    MOVE_TOWARD(delay, 0.1);
    MOVE_TOWARD(waste_T, 0.1);
    MOVE_TOWARD(waste_I, 0.1);
};

inline Winner match(eval::Weight w1, eval::Weight w2, i32 iter, SaveData& save)
{
    auto queue = battle::generate_queue();
    
    printf("Matching w+ vs w-...\n");

    auto r = battle::do_battle(w1, w2, queue, queue, iter);

    auto r1 = std::get<0>(r);
    auto r2 = std::get<1>(r);

    auto cmp_result = [] (const battle::Result& a, const battle::Result& b) -> bool {
        if (a.win || b.win) {
            return a.win < b.win;
        }

        if (a.attack != b.attack) {
            return a.attack < b.attack;
        }

        return a.max_spike < b.max_spike;
    };

    save.avg = (r1.attack + r2.attack) / 2;

    std::cout << "\n";

    std::cout << "P1:\t" << r1.attack << "\tSPIKE:\t" << r1.max_spike << "\n";
    std::cout << "P2:\t" << r2.attack << "\tSPIKE:\t" << r2.max_spike << "\n";
    std::cout << "Avg:\t" << save.avg << "\n";

    if (cmp_result(r2, r1)) {
        save.winner = r1.attack;
        save.loser = r2.attack;

        return Winner::P1;
    }

    if (cmp_result(r1, r2)) {
        save.winner = r2.attack;
        save.loser = r1.attack;

        return Winner::P2;
    }

    save.winner = r1.attack;
    save.loser = r2.attack;

    return Winner::DRAW;
};

inline void run(eval::Weight w, i32 id_init = 0)
{
    system("cls");

    i32 id = id_init;

    while (true)
    {
        auto [w1, w2] = tuner::randomize(w, id);

        SaveData save = { 0 };

        auto winner = tuner::match(w1, w2, 3600 * 30, save);

        if (winner == Winner::P1) {
            tuner::move_toward(w, w1, id);
        }
        else if (winner == Winner::P2) {
            tuner::move_toward(w, w2, id);
        }
        else {
            continue;
        }

        save.w = w;

        tuner::save(std::to_string(id), save);

        printf("id: %d\n\n", id);

        id += 1;
    }
};

};