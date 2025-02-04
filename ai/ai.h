#pragma once

#include "beam.h"
#include "path.h"

namespace ai
{

struct Plan
{
    move::Placement placement = move::Placement();
    State root = State();
    u32 eval = 0;
    u32 nodes = 0;
    u32 depth = 0;
};

class Engine
{
private:
    std::mutex mutex;
    std::atomic_flag running;
    std::thread* thread;
public:
    State state;
    Lock lock;
    std::vector<piece::Type> queue;
    eval::Weight w;
    std::optional<beam::Result> result;
public:
    Engine();
public:
    bool init(std::vector<piece::Type> queue, State state, Lock lock, eval::Weight w);
    bool advance(move::Placement placement, std::vector<piece::Type> next);
    bool reset(Board board, i32 b2b, i32 ren);
public:
    bool search(beam::Configs configs = beam::Configs());
    std::optional<Plan> request(i32 incomming_attack);
public:
    void clear();
    bool is_running();
};

};