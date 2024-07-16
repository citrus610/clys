#include "ai.h"

namespace AI
{

Engine::Engine()
{
    this->clear();
};

bool Engine::init(std::vector<Piece::Type> queue, Lock lock, State state, Evaluation::Weight w)
{
    if (this->thread != nullptr || this->flag_running.test()) {
        return false;
    }

    this->clear();

    this->root_state = state;
    this->root_lock = lock;
    this->queue = queue;
    this->w = w;

    return true;
};

bool Engine::advance(Piece::Data placement, std::vector<Piece::Type> next)
{
    if (this->thread != nullptr || this->flag_running.test()) {
        return false;
    }

    Bag bag_end_queue = this->root_state.bag;
    for (auto& p : this->queue) bag_end_queue.update(p);

    if (!Beam::is_queue_valid(next, bag_end_queue)) {
        return false;
    }

    this->root_lock = this->root_state.advance(placement, this->queue);

    this->queue.erase(this->queue.begin(), this->queue.begin() + this->root_state.next);
    this->queue.insert(this->queue.end(), next.begin(), next.end());

    this->root_state.next = 0;

    this->result = std::nullopt;

    return true;
};

bool Engine::reset(Board board, i32 b2b, i32 ren)
{
    if (this->thread != nullptr || this->flag_running.test()) {
        return false;
    }

    this->root_state.board = board;
    this->root_state.b2b = b2b;
    this->root_state.ren = ren;

    this->result = std::nullopt;

    return true;
};

bool Engine::search(size_t width, size_t fwidth, size_t depth_max)
{
    if (this->thread != nullptr || this->flag_running.test()) {
        return false;
    }

    this->flag_running.test_and_set();

    this->thread = new std::thread([&] (std::vector<Piece::Type> squeue, State sstate, Lock slock, size_t swidth, size_t sfwidth, size_t sdepth_max) {
        Beam::Result sresult = Beam::Result();

        Node::Data root = Node::Data();
        root.state = sstate;
        root.state.next = 0;
        root.lock = slock;

        Layer layers[2] = { Layer(), Layer() };
        layers[0].init(swidth);
        layers[1].init(swidth);

        Move::Generator generator = Move::Generator();

        Beam::init_candidates(sresult.candidates, squeue, root, layers[0], generator, this->w);

        if (sresult.candidates.empty()) {
            std::lock_guard<std::mutex> lk(mutex);
            this->result = sresult;
            return;
        }

        sresult.depth = 1;

        for (auto i = 0; i < squeue.size() - 1 - (root.state.hold == Piece::Type::NONE); ++i) {
            i32 index = Beam::think(
                squeue,
                sresult.candidates,
                layers[(sresult.depth - 1) & 1],
                layers[sresult.depth & 1],
                generator,
                this->w,
                sresult.nodes,
                sresult.depth,
                sresult.pc
            );

            if (index != -1) {
                std::lock_guard<std::mutex> lk(mutex);

                sresult.candidates[index].visit += sdepth_max * swidth;

                this->result = sresult;
            }

            sresult.depth++;
        }

        const size_t branch_count = 3;

        for (auto& c : sresult.candidates) {
            c.visit *= branch_count;
        }

        Bag bag_end_queue = root.state.bag;
        for (auto& p : squeue) bag_end_queue.update(p);

        std::vector<Piece::Type> fqueue[branch_count];

        for (auto i = 0; i < branch_count; ++i) {
            fqueue[i] = squeue;

            auto fbag_end_queue = bag_end_queue;

            while (true)
            {
                auto queue_next = Beam::get_queue_monte_carlo(fbag_end_queue);
                fqueue[i].insert(fqueue[i].end(), queue_next.begin(), queue_next.end());

                fbag_end_queue = Bag();

                if (fqueue[i].size() >= sdepth_max) {
                    break;
                }
            }
        }

        Layer flayers[branch_count][2];
        
        for (auto i = 0; i < branch_count; ++i) {
            flayers[i][0] = Layer();
            flayers[i][1] = Layer();
            flayers[i][0].init(sfwidth);
            flayers[i][1].init(sfwidth);

            flayers[i][(sresult.depth - 1) & 1].data = layers[(sresult.depth - 1) & 1].data;
        }

        i32 findex[branch_count];

        for (i32 i = 0; i < branch_count; ++i) {
            findex[i] = -1;
        }

        while (this->flag_running.test() && sresult.depth < sdepth_max - 2)
        {
            for (auto i = 0; i < branch_count; ++i) {
                if (findex[i] != -1) {
                    sresult.candidates[findex[i]].visit += swidth;
                }
                else {
                    bool pc = false;

                    findex[i] = Beam::think(
                        fqueue[i],
                        sresult.candidates,
                        flayers[i][(sresult.depth - 1) & 1],
                        flayers[i][sresult.depth & 1],
                        generator,
                        this->w,
                        sresult.nodes,
                        sresult.depth,
                        pc
                    );
                }
            }

            sresult.depth++;
        }

        std::lock_guard<std::mutex> lk(mutex);
        this->result = sresult;
    }, this->queue, this->root_state, this->root_lock, width, fwidth, depth_max);

    return true;
};

std::optional<Plan> Engine::request(i32 incomming_attack)
{
    if (!this->flag_running.test() || this->thread == nullptr) {
        return {};
    }

    this->flag_running.clear();

    if (this->thread->joinable()) {
        this->thread->join();
    }

    delete this->thread;

    this->thread = nullptr;

    if (!this->result.has_value() || this->result.value().candidates.empty()) {
        return {};
    }

    auto bresult = this->result.value();
    this->result = {};

    std::sort(
        bresult.candidates.begin(),
        bresult.candidates.end(),
        [&] (const Beam::Candidate& a, const Beam::Candidate& b) {
            return b < a;
        }
    );

    size_t best_index = 0;

    i32 heights[10];
    this->root_state.board.get_heights(heights);
    i32 height_center_max = *std::max_element(heights + 3, heights + 7);

    for (size_t i = 0; i < bresult.candidates.size(); ++i) {
        if (height_center_max + incomming_attack - bresult.candidates[i].spike <= 20) {
            best_index = i;
            break;
        }
    }

    u32 total_visit = 0;
    for (auto& c : bresult.candidates) {
        total_visit += c.visit;
    }

    double e_sum = 0.0;
    double e_adjust = 4.0;
    for (auto& c : bresult.candidates) {
        e_sum += std::exp(double(c.visit) / double(total_visit) * e_adjust);
    }

    double eval = std::exp(double(bresult.candidates[best_index].visit) / double(total_visit) * e_adjust) / e_sum;

    return Plan {
        .placement = bresult.candidates[best_index].placement,
        .path = Path::search(this->root_state.board, bresult.candidates[best_index].placement),
        .root = this->root_state,
        .eval = eval,
        .nodes = bresult.nodes,
        .depth = bresult.depth,
        .pc = bresult.pc
    };
};

void Engine::clear()
{
    this->flag_running.clear();
    this->thread = nullptr;

    this->root_state = State();
    this->root_lock = Lock();
    this->queue.clear();
    this->w = Evaluation::Weight();
    this->result = {};
};

bool Engine::is_running()
{
    return this->flag_running.test();
};

};