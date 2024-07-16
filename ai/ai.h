#pragma once

#include "beam.h"
#include "path.h"
#include "gaze.h"

namespace AI
{

struct Plan
{
    Piece::Data placement = Piece::Data();
    std::vector<Path::Move> path = {};
    State root = State();
    double eval = 0;
    u32 nodes = 0;
    u32 depth = 0;
    bool pc = false;
};

class Engine
{
private:
    std::mutex mutex;
    std::atomic_flag flag_running;
    std::thread* thread;
public:
    State root_state;
    Lock root_lock;
    std::vector<Piece::Type> queue;
    Evaluation::Weight w;
    std::optional<Beam::Result> result;
public:
    Engine();
public:
    bool init(
        std::vector<Piece::Type> queue,
        Lock lock = Lock(),
        State state = State(),
        Evaluation::Weight w = Evaluation::DEFAULT
    );
    bool advance(Piece::Data placement, std::vector<Piece::Type> next);
    bool reset(Board board, i32 b2b, i32 ren);
public:
    bool search(size_t width = Beam::WIDTH_NORMAL, size_t fwidth = Beam::WIDTH_NORMAL, size_t depth_max = 32);
    std::optional<Plan> request(i32 incomming_attack);
public:
    void clear();
    bool is_running();
};

};