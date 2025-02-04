#pragma once

#include "../core/state.h"

namespace node
{

struct Score
{
    i32 action = 0;
    i32 eval = 0;
};

struct Data
{
    State state = State();
    Score score = Score();
    Lock lock = Lock();
    i32 index = -1;
};

inline bool operator < (const Score& a, const Score& b)
{
    return a.action + a.eval < b.action + b.eval;
};

inline bool operator < (const node::Data& a, const node::Data& b)
{
    return a.score < b.score;
};

};