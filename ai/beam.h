#pragma once

#include "layer.h"
#include "eval.h"

namespace beam
{

struct Configs
{
    size_t width = 500;
    size_t depth = 32;
    size_t branch = 3;
};

struct Candidate
{
    move::Placement placement = move::Placement();
    size_t visit = 0;
};

struct Result
{
    std::vector<Candidate> candidates;
    u32 nodes = 0;
    u32 depth = 0;
};

u32 expand(
    std::vector<piece::Type>& queue,
    node::Data& node,
    std::function<void(node::Data&, move::Placement)> callback
);

i32 think(
    std::vector<piece::Type>& queue,
    std::vector<Candidate>& candidates,
    Layer& parents,
    Layer& children,
    const eval::Weight& w,
    u32& nodes
);

Result search(
    State state,
    Lock lock,
    std::vector<piece::Type> queue,
    eval::Weight w,
    Configs configs,
    std::atomic_flag& running
);

std::vector<piece::Type> get_queue_random(Bag bag, size_t count);

bool is_queue_valid(const std::vector<piece::Type>& queue, Bag bag);

inline bool operator < (const Candidate& a, const Candidate& b)
{
    return a.visit < b.visit;
};

};