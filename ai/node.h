#pragma once

#include "../core/state.h"

namespace Node
{

struct Score
{
    i32 acml = 0;
    i32 eval = 0;
};

struct Data
{
    State state = State();
    Score score = Score();
    Lock lock = Lock();
    i32 index = -1;
};

static bool operator < (const Score& a, const Score& b)
{
    return a.acml + a.eval < b.acml + b.eval;
};

static bool operator < (const Node::Data& a, const Node::Data& b)
{
    return a.score < b.score;
};

};