#include "ai.h"

namespace ai
{

Engine::Engine()
{
    this->clear();
};

// Initializes the search engine
bool Engine::init(std::vector<piece::Type> queue, State state, Lock lock, eval::Weight w)
{
    if (this->thread != nullptr || this->running.test()) {
        return false;
    }

    this->clear();

    this->state = state;
    this->lock = lock;
    this->queue = queue;
    this->w = w;
    this->result = std::nullopt;

    return true;
};

// Advances to the next state of the search
bool Engine::advance(move::Placement placement, std::vector<piece::Type> next)
{
    if (this->thread != nullptr || this->running.test()) {
        return false;
    }

    auto bag = this->state.bag;

    for (auto& p : this->queue) {
        bag.update(p);
    }

    if (!beam::is_queue_valid(next, bag)) {
        return false;
    }

    this->lock = this->state.advance(placement, this->queue);

    this->queue.erase(this->queue.begin(), this->queue.begin() + this->state.next);
    this->queue.insert(this->queue.end(), next.begin(), next.end());

    this->state.next = 0;

    this->result = std::nullopt;

    return true;
};

// Resets the search state
// Call this function after garbages were placed or bot misdroped
bool Engine::reset(Board board, i32 b2b, i32 ren)
{
    if (this->thread != nullptr || this->running.test()) {
        return false;
    }

    this->state.board = board;
    this->state.b2b = b2b;
    this->state.ren = ren;

    this->result = std::nullopt;

    return true;
};

// Starts the search thread
bool Engine::search(beam::Configs configs)
{
    if (this->thread != nullptr || this->running.test()) {
        return false;
    }

    this->result = std::nullopt;
    this->running.test_and_set();

    this->thread = new std::thread(
        [&] (beam::Configs cfs) {
            auto beam_result = beam::search(this->state, this->lock, this->queue, this->w, cfs, this->running);

            std::lock_guard<std::mutex> lk(mutex);

            this->result = beam_result;
        },
        configs
    );

    return true;
};

// Requests move from the bot and stops the search thread
std::optional<Plan> Engine::request(i32 incomming_attack)
{
    if (!this->running.test() || this->thread == nullptr) {
        return {};
    }

    // Stops the running flag atomically
    this->running.clear();

    // Joins thread
    if (this->thread->joinable()) {
        this->thread->join();
    }

    delete this->thread;

    this->thread = nullptr;

    // Checks result
    if (!this->result.has_value() || this->result.value().candidates.empty()) {
        return {};
    }

    auto beam_result = this->result.value();

    this->result = {};

    // Sorts the result based on the visit count
    std::sort(
        beam_result.candidates.begin(),
        beam_result.candidates.end(),
        [&] (const beam::Candidate& a, const beam::Candidate& b) {
            return b < a;
        }
    );

    size_t best_index = 0;

    for (size_t i = 0; i < beam_result.candidates.size(); ++i) {
        auto simulate_state = this->state;
        auto simulate_lock = simulate_state.advance(beam_result.candidates[i].placement, this->queue);

        i32 heights[10];
        simulate_state.board.get_heights(heights);
        
        i32 height_center_max = *std::max_element(heights + 3, heights + 7);

        if (height_center_max + incomming_attack - simulate_lock.attack - simulate_lock.clear <= 20) {
            best_index = i;
            break;
        }
    }

    return Plan {
        .placement = beam_result.candidates[best_index].placement,
        .root = this->state,
        .eval = u32(beam_result.candidates[best_index].visit),
        .nodes = beam_result.nodes,
        .depth = beam_result.depth
    };
};

void Engine::clear()
{
    this->running.clear();
    this->thread = nullptr;

    this->state = State();
    this->lock = Lock();
    this->queue.clear();
    this->w = eval::Weight();
    this->result = {};
};

bool Engine::is_running()
{
    return this->running.test();
};

};