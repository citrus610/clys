#pragma once

#include "layer.h"
#include "move.h"
#include "eval.h"

namespace Beam
{

constexpr size_t WIDTH_NORMAL = 400;
constexpr size_t WIDTH_FUTURE = 250;

struct Candidate
{
    Piece::Data placement = Piece::Data();
    i32 spike = 0;
    u32 visit = 0;
};

struct Result
{
    std::vector<Candidate> candidates;
    u32 nodes = 0;
    u32 depth = 0;
    bool pc = false;
};

Result search(
    std::vector<Piece::Type> queue,
    State root_state = State(),
    Lock root_lock = Lock(),
    Evaluation::Weight w = Evaluation::DEFAULT,
    size_t width = WIDTH_NORMAL
);

void expand(
    std::vector<Piece::Type>& queue,
    Node::Data& node,
    Layer& layer,
    Move::Generator& generator,
    Evaluation::Weight& w,
    u32& nodes
);

i32 think(
    std::vector<Piece::Type>& queue,
    std::vector<Candidate>& candidates,
    Layer& old_layer,
    Layer& new_layer,
    Move::Generator& generator,
    Evaluation::Weight& w,
    u32& nodes,
    u32 depth,
    bool& pc
);

void init_candidates(
    std::vector<Candidate>& candidates,
    std::vector<Piece::Type>& queue,
    Node::Data& root,
    Layer& layer,
    Move::Generator& generator,
    Evaluation::Weight& w
);

std::vector<Piece::Type> get_queue_monte_carlo(Bag bag);

u32 get_attack(State& state, Lock& lock);

i32 get_candidate_spike(State& state, Lock& lock);

bool is_queue_valid(std::vector<Piece::Type>& queue, Bag bag);

static void shuffle_queue(std::vector<Piece::Type>& queue)
{
    size_t n = queue.size();

    for (size_t i = 0; i < n; ++i) {
        size_t k = size_t(rand()) % n;
        std::swap(queue[i], queue[k]);
    }
};

static bool operator < (const Candidate& a, const Candidate& b)
{
    return a.visit < b.visit;
};

};