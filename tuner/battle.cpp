#include "battle.h"

namespace battle
{

Sync::Sync()
{
    this->clear();
};

void Sync::clear()
{
    this->counter = -1;
};

void Sync::wait(i32 frame)
{
    this->counter = frame;
};

bool Sync::is_waiting()
{
    this->counter = std::max(this->counter - 1, -1);

    return this->counter > -1;
};

Player::Player()
{
    this->ai.clear();
    this->plan = {};
    this->w = eval::Weight();

    this->sync = Sync();
    this->delay = Delay();

    this->state = State();
    this->queue = {};
    this->garbages = {};
    this->queue_index = 0;
    this->running = false;

    this->attack_total = 0;
    this->pc = 0;
    this->tspin = 0;
    this->tetris = 0;
    this->ren = {};
};

void Player::init(eval::Weight w, std::vector<piece::Type> q)
{
    this->plan = {};
    this->w = w;

    this->sync = Sync();
    this->delay = Delay();

    this->state = State();
    this->queue = q;
    this->garbages = {};
    this->running = true;

    this->attack_total = 0;
    this->pc = 0;
    this->tspin = 0;
    this->tetris = 0;
    this->ren = {};

    auto init_queue = { this->queue[0], this->queue[1], this->queue[2], this->queue[3], this->queue[4] };
    this->queue_index = 5;

    this->sync.wait(this->delay.start);

    this->ai.clear();
    this->ai.init(init_queue, State(), Lock(), this->w);
    this->ai.search();
};

void Player::update(Player* enemy)
{
    if (!this->running) {
        return;
    }

    for (auto& g : this->garbages) {
        g.frame = std::max(0, g.frame - 1);
    }

    if (this->sync.is_waiting()) {
        return;
    }

    if (this->plan.has_value()) {
        // Advance state
        auto state_previous = this->state;
        auto lock = this->state.advance(this->plan.value().placement, this->ai.queue);
        this->state.next = 0;

        // Update log
        if (lock.clear > 0) {
            this->pc += this->state.board.is_empty();

            if (lock.tspin) {
                this->tspin += lock.clear;
            }

            if (lock.clear == 4) {
                this->tetris += 1;
            }
        }

        if (state_previous.ren > this->state.ren && state_previous.ren > 1) {
            this->ren.push_back(state_previous.ren - 1);
        }

        // Calculate attack
        i32 attack = lock.attack;

        this->attack_total += attack;

        // Update garbage
        while (attack > 0 && !this->garbages.empty())
        {
            if (attack >= this->garbages[0].count) {
                attack -= this->garbages[0].count;
                this->garbages.erase(this->garbages.begin());
                continue;
            }

            this->garbages[0].count -= attack;
            attack = 0;
        }

        // Send attack
        if (attack > 0) {
            if (enemy->garbages.empty()) {
                enemy->garbages.push_back(battle::Garbage {
                    .frame = this->delay.garbage,
                    .count = attack
                });
            }
            else {
                enemy->garbages[0].count += attack;
            }
        }

        // Place garbage
        if (!this->garbages.empty()) {
            i32 garbage = 0;

            while (true)
            {
                if (this->garbages[0].frame != 0) {
                    break;
                }

                garbage += this->garbages[0].count;
                this->garbages.erase(this->garbages.begin());

                if (this->garbages.empty()) {
                    break;
                }
            }

            battle::place_garbage(this->state.board, garbage);
        }

        // Clear plan
        this->plan = {};
    }

    // Request from AI
    i32 garbage = 0;

    for (auto& g : this->garbages) {
        garbage += g.count;
    }

    auto r = this->ai.request(garbage);

    // Die
    if (!r.has_value()) {
        this->running = false;
        return;
    }

    // Check valid
    if (this->state.board != r->root.board) {
        this->ai.reset(this->state.board, this->state.b2b, this->state.ren);
        this->ai.search({ .width = 250, .depth = 32, .branch = 1 });
        this->sync.wait(2);
        return;
    }

    // Advance AI
    std::vector<piece::Type> next = { this->queue[this->queue_index] };
    this->queue_index = (this->queue_index + 1) % this->queue.size();

    bool first_hold = (this->state.hold == piece::Type::NONE) && (r.value().placement.type != ai.queue.front());

    if (first_hold) {
        next.push_back(this->queue[this->queue_index]);
        this->queue_index = (this->queue_index + 1) % this->queue.size();
    }

    auto state_copy = this->state;
    auto lock = state_copy.advance(r.value().placement, this->ai.queue);

    if (!this->ai.advance(r.value().placement, next)) {
        this->running = false;
        return;
    }

    this->ai.search();

    // Calculate delay
    i32 delay = this->delay.spawn;

    delay += std::abs(i32(r.value().placement.x) - 4) * this->delay.das;

    if (lock.softdrop) {
        delay += std::max(0, 19 - i32(r.value().placement.y)) * this->delay.softdrop;
    }

    if (lock.clear > 0) {
        if (state_copy.board.is_empty()) {
            delay += this->delay.pc;
        }
        else {
            delay += this->delay.clear[lock.clear - 1];
        }
    }

    // Operating
    this->plan = r;
    this->sync.wait(delay);
};

void Player::end()
{
    this->ai.clear();
};

bool Player::is_dead()
{
    return !this->running;
};

void Game::init(eval::Weight w1, eval::Weight w2, std::vector<piece::Type> q1, std::vector<piece::Type> q2)
{
    this->players[0].init(w1, q1);
    this->players[1].init(w2, q2);
};

void Game::update()
{
    this->players[0].update(&this->players[1]);
    this->players[1].update(&this->players[0]);
};

void Game::end()
{
    this->players[0].end();
    this->players[1].end();
};

};