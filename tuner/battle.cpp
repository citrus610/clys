#include "battle.h"

namespace Battle
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
    this->counter -= 1;

    return this->counter > -1;
};

Player::Player()
{
    this->ai.clear();
    this->plan = {};

    this->sync = Sync();
    this->delay = Delay();

    this->state = State();
    this->queue.clear();
    this->garbages.clear();
    this->queue_index = 0;
    this->running = false;

    this->attack_total = 0;
    this->pc = 0;
    this->tspin = 0;
};

void Player::init(Evaluation::Weight w)
{
    this->running = true;

    this->w = w;
    this->plan = {};

    this->sync = Sync();
    this->delay = Delay();

    this->garbages.clear();

    this->state = State();

    this->queue = generate_queue();

    auto init_queue = { this->queue[0], this->queue[1], this->queue[2], this->queue[3], this->queue[4] };
    this->queue_index = 5;

    this->sync.wait(this->delay.start);

    this->attack_total = 0;
    this->pc = 0;
    this->tspin = 0;

    this->ai.clear();
    this->ai.init(init_queue, Lock(), State(), this->w);
    this->ai.search();
};

void Player::update(Player* enemy)
{
    if (!this->running) {
        return;
    }

    // Update garbage
    for (auto& g : this->garbages) {
        g.frame = std::max(0, g.frame - 1);
    }

    // Skip if is waiting
    if (this->sync.is_waiting()) {
        return;
    }

    // Place piece and update state
    if (this->plan.has_value()) {
        // Advance state
        auto lock = this->state.advance(this->plan.value().placement, this->ai.queue);
        this->state.next = 0;

        // Update log
        this->pc += lock.pc;

        if (lock.tspin) {
            this->tspin += lock.clear;
        }

        // Calculate attack
        i32 attack = Beam::get_attack(this->state, lock);

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
            enemy->garbages.push_back(Battle::Garbage {
                .frame = this->delay.garbage,
                .count = attack
            });

            // this->attack_total += attack;
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

            Battle::place_garbage(this->state.board, std::clamp(garbage, 0, 10));
        }

        // Clear plan
        this->plan = {};

        // Delay line clear
        i32 delay = 0;

        if (lock.clear > 0) {
            if (lock.pc) {
                delay += this->delay.pc;
            }
            else {
                delay += this->delay.clear[lock.clear - 1];
            }
        }

        if (delay > 0) {
            this->sync.wait(delay);
            return;
        }
    }

    // Request from AI
    i32 garbage = 0;

    for (auto& g : this->garbages) {
        garbage += g.count;
    }

    auto r = this->ai.request(std::clamp(garbage, 0, 10));

    // Die
    if (!r.has_value()) {
        this->running = false;
        return;
    }

    // Check valid
    if (this->state.board != r->root.board) {
        this->ai.reset(this->state.board, this->state.b2b, this->state.ren);
        this->ai.search();
        this->sync.wait(1);
        return;
    }

    // Advance AI
    std::vector<Piece::Type> next = { this->queue[this->queue_index] };
    this->queue_index += 1;
    this->queue_index = this->queue_index % this->queue.size();

    bool first_hold = this->state.hold == Piece::Type::NONE && r.value().placement.type != ai.queue.front();

    if (first_hold) {
        next.push_back(this->queue[this->queue_index]);
        this->queue_index += 1;
        this->queue_index = this->queue_index % this->queue.size();
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

void Game::init(Evaluation::Weight w1, Evaluation::Weight w2)
{
    this->players[0].init(w1);
    this->players[1].init(w2);
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