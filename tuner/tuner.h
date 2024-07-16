#pragma once

#include <fstream>

#include "score.h"
#include "battle.h"

namespace Tuner
{

struct SaveData {
    Evaluation::Weight w;
    Score score;
    i32 unchange;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, w, score, unchange)

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

static void print_w(Evaluation::Weight w)
{
    #define PRW(p) printf("%s: %d\n", #p, w.p);

    PRW(resource);
    PRW(well);
    PRW(structure[0]);
    PRW(structure[1]);
    PRW(structure[2]);
    PRW(b2b);
    PRW(ren_bonus);
    PRW(parity);
    PRW(parity_v);
    PRW(ren[0]);
    PRW(ren[1]);
    PRW(ren[2]);
    PRW(tspin[0]);
    PRW(tspin[1]);

    PRW(clear[0]);
    PRW(clear[1]);
    PRW(clear[2]);
    PRW(break_b2b);
    PRW(waste_T);
};

Evaluation::Weight constrain(Evaluation::Weight w)
{
    #define CONSTRAIN_POSITIVE(p) w.p = std::max(0, w.p);
    #define CONSTRAIN_NEGATIVE(p) w.p = std::min(0, w.p);

    CONSTRAIN_POSITIVE(resource);
    CONSTRAIN_POSITIVE(well);
    CONSTRAIN_POSITIVE(structure[0]);
    CONSTRAIN_POSITIVE(structure[1]);
    CONSTRAIN_POSITIVE(structure[2]);
    CONSTRAIN_POSITIVE(b2b);
    CONSTRAIN_POSITIVE(ren_bonus);
    CONSTRAIN_POSITIVE(ren[0]);
    CONSTRAIN_POSITIVE(ren[1]);
    CONSTRAIN_POSITIVE(ren[2]);
    CONSTRAIN_POSITIVE(tspin[0]);
    CONSTRAIN_POSITIVE(tspin[1]);

    CONSTRAIN_NEGATIVE(border);
    CONSTRAIN_NEGATIVE(bump_s);
    CONSTRAIN_NEGATIVE(cover);
    CONSTRAIN_NEGATIVE(parity);
    CONSTRAIN_NEGATIVE(parity_v);
    CONSTRAIN_NEGATIVE(hole);
    CONSTRAIN_NEGATIVE(clear[0]);
    CONSTRAIN_NEGATIVE(clear[1]);
    CONSTRAIN_NEGATIVE(clear[2]);
    CONSTRAIN_NEGATIVE(break_b2b);
    CONSTRAIN_NEGATIVE(waste_T);

    return w;
};

std::pair<Evaluation::Weight, Evaluation::Weight> randomize(Evaluation::Weight w)
{
    auto w1 = w;
    auto w2 = w;
    auto w_pre = w;

    i32* param[] = {
        &w.border,
        &w.bump_s,
        &w.resource,
        &w.cover,
        &w.parity,
        &w.parity_v,
        &w.hole,
        &w.structure[0],
        &w.structure[1],
        &w.structure[2],
        &w.clear[0],
        &w.clear[1],
        &w.clear[2],
        &w.ren[0],
        &w.ren[1],
        &w.ren_bonus,
        &w.break_b2b,
        &w.waste_T,
    };

    i32 param_delta[_countof(param)] = { 0 };

    for (i32 i = 0; i < _countof(param); ++i) {
        i32 delta = 50;
        if (i < 5) delta = 15;
        if (i == 1) delta = 10;
        if (i == 5) delta = 20;

        i32 sign = (rand() % 2) * 2 - 1;
        i32 value = 5 + (rand() % delta);
        if (i < 5) value = 2 + (rand() % delta);

        param_delta[i] = value * sign;
    }

    for (i32 i = 0; i < _countof(param); ++i) {
        *param[i] += param_delta[i];
    }

    w1 = constrain(w);

    w = w_pre;

    for (i32 i = 0; i < _countof(param); ++i) {
        *param[i] -= param_delta[i];
    }

    w2 = constrain(w);

    return { w1, w2 };
};

std::vector<Piece::Type> gen_rng_queue()
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

    for (i32 i = 0; i < 1000; i++) {
        auto full_cp = full;
        std::shuffle(full_cp.begin(), full_cp.end(), rng);
        for (auto p : full_cp) {
            result.push_back(p);
        }
    }

    return result;
};

i32 match(Evaluation::Weight w1, Evaluation::Weight w2, Score& s1, Score& s2)
{
    // auto q = Tuner::gen_rng_queue();
    // auto g = Tuner::generate_garbage();

    // printf("\nsimulating w+ and w-:\n");

    // auto thread_1 = std::thread([&] () {
    //     s1 = Tuner::get_score(q, g, w1);
    // });

    // auto thread_2 = std::thread([&] () {
    //     s2 = Tuner::get_score(q, g, w2);
    // });

    // thread_1.join();
    // thread_2.join();

    // if (s1 < s2) {
    //     return -1;
    // }

    // if (s2 < s1) {
    //     return 1;
    // }
    
    // return 0;

    printf("Matching w+ vs w-...\n");

    struct Result
    {
        bool win = false;
        i32 attack = 0;
        i32 pc = 0;
        i32 tspin = 0;
    };

    auto cmp_result = [&] (const Result& a, const Result& b) -> bool {
        if (a.win != b.win) {
            return a.win < b.win;
        }

        if (std::abs(a.attack - b.attack) > 2) {
            return a.attack < b.attack;
        }

        if (a.pc != b.pc) {
            return a.pc < b.pc;
        }

        if (a.tspin != b.tspin) {
            return a.tspin < b.tspin;
        }

        return a.attack < b.attack;
    };

    Battle::Game game;

    game.init(w1, w2);

    i32 frame = 0;

    Result r1 = Result();
    Result r2 = Result();

    while (true)
    {
        game.update();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        frame += 1;
        printf("\rframe: %d        ", frame);

        if (game.players[0].is_dead() || game.players[1].is_dead() || frame >= 3600) {
            r1 = Result {
                .win = game.players[1].is_dead(),
                .attack = game.players[0].attack_total,
                .pc = game.players[0].pc,
                .tspin = game.players[0].tspin
            };

            r2 = Result {
                .win = game.players[0].is_dead(),
                .attack = game.players[1].attack_total,
                .pc = game.players[1].pc,
                .tspin = game.players[1].tspin
            };

            break;
        }
    };

    game.end();

    printf("\n");

    system("cls");

    printf("w+: win - %d | attack - %d\n", r1.win, r1.attack);
    printf("w-: win - %d | attack - %d\n", r2.win, r2.attack);

    if (cmp_result(r2, r1)) {
        return 1;
    }

    if (cmp_result(r1, r2)) {
        return -1;
    }

    return 0;
};

void move_toward(Evaluation::Weight& w, Evaluation::Weight target)
{
    #define MOVE_TOWARD(p, r) w.p += i32(std::round(double(target.p - w.p) * r));

    MOVE_TOWARD(height, 0.1);
    MOVE_TOWARD(height_10, 0.1);
    MOVE_TOWARD(height_15, 0.1);
    MOVE_TOWARD(bump, 0.1);
    MOVE_TOWARD(bump_s, 0.3);
    MOVE_TOWARD(resource, 0.1);
    MOVE_TOWARD(parity, 0.1);
    MOVE_TOWARD(parity_v, 0.1);
    MOVE_TOWARD(border, 0.1);
    MOVE_TOWARD(hole, 0.1);
    MOVE_TOWARD(cover, 0.1);
    MOVE_TOWARD(well, 0.5);
    MOVE_TOWARD(well_x, 0.1);
    MOVE_TOWARD(structure[0], 0.1);
    MOVE_TOWARD(structure[1], 0.1);
    MOVE_TOWARD(structure[2], 0.1);
    MOVE_TOWARD(structure[3], 0.1);
    MOVE_TOWARD(have_b2b, 0.1);

    MOVE_TOWARD(tspin[0], 0.1);
    MOVE_TOWARD(tspin[1], 0.1);
    MOVE_TOWARD(tspin[2], 0.1);
    MOVE_TOWARD(clear[0], 0.1);
    MOVE_TOWARD(clear[1], 0.1);
    MOVE_TOWARD(clear[2], 0.1);
    MOVE_TOWARD(clear[3], 0.1);
    MOVE_TOWARD(break_b2b, 0.1);
    MOVE_TOWARD(b2b, 0.1);
    MOVE_TOWARD(ren[0], 0.1);
    MOVE_TOWARD(ren[1], 0.1);
    MOVE_TOWARD(ren[2], 0.1);
    MOVE_TOWARD(ren[3], 0.1);
    MOVE_TOWARD(ren[4], 0.1);
    MOVE_TOWARD(ren_bonus, 0.1);
    MOVE_TOWARD(waste_time, 0.1);
    MOVE_TOWARD(waste_T, 0.1);
};

static void run(Evaluation::Weight w)
{
    system("cls");

    i32 unchange = 0;

    i32 id = 0;

    while (true)
    {
        auto randw = Tuner::randomize(w);

        auto w1 = randw.first;
        auto w2 = randw.second;

        Score s1, s2;

        auto m = Tuner::match(w1, w2, s1, s2);

        if (m == 1) {
            unchange = 0;
            Tuner::move_toward(w, w1);
        }
        else if (m == -1) {
            unchange = 0;
            Tuner::move_toward(w, w2);
        }
        else {
            unchange += 1;

            if (unchange >= 10) {
                break;
            }

            if (id > 0) {
                SaveData save_data;

                Tuner::load(std::to_string(id - 1), save_data);
                save_data.unchange = unchange;
                Tuner::save(std::to_string(id - 1), save_data);
            }

            continue;
        }

        // printf("\nsimulating new w:\n");
        // auto new_s = Tuner::get_score(Tuner::gen_rng_queue(), Tuner::generate_garbage(), w);

        auto save_data = SaveData{
            .w = w,
            .score = 0,
            .unchange = 0,
        };

        Tuner::save(std::to_string(id), save_data);

        // system("cls");
        printf("id: %d\n\n", id);
        // printf("w+: %d - %d - %d - %d\n", s1.attack, s1.pc, s1.ren, s1.tspin);
        // printf("w-: %d - %d - %d - %d\n", s2.attack, s2.pc, s2.ren, s2.tspin);
        // printf("w0: %d - %d - %d - %d\n", new_s.attack, new_s.pc, new_s.ren, new_s.tspin);
        // printf("\n");
        // Tuner::print_w(w);

        id += 1;
    }
};

};