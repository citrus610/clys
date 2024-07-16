#include "beam.h"

namespace Beam
{

Result search(
    std::vector<Piece::Type> queue,
    State root_state,
    Lock root_lock,
    Evaluation::Weight w,
    size_t width
) {
    Result result = Result();

    if (queue.size() < 2 || !Beam::is_queue_valid(queue, root_state.bag)) {
        return result;
    }

    Node::Data root = Node::Data();
    root.state = root_state;
    root.state.next = 0;
    root.lock = root_lock;

    Layer layers[2] = { Layer(), Layer() };
    layers[0].init(width);
    layers[1].init(width);

    Move::Generator generator = Move::Generator();

    Beam::init_candidates(result.candidates, queue, root, layers[0], generator, w);

    result.depth = 1;

    for (size_t i = 0; i < queue.size() - 1 - (root.state.hold == Piece::Type::NONE); ++i) {
        Beam::think(queue, result.candidates, layers[(result.depth - 1) & 1], layers[result.depth & 1], generator, w, result.nodes, result.depth, result.pc);
        result.depth++;
    }

    // auto& last = layers[(result.depth - 1) & 1];

    // for (auto& node : last.data) {
    //     result.candidates[node.index].eval = std::max(result.candidates[node.index].eval, node.score.acml + node.score.eval);
    // }

    return result;
};

void expand(
    std::vector<Piece::Type>& queue,
    Node::Data& node,
    Layer& layer,
    Move::Generator& generator,
    Evaluation::Weight& w,
    u32& nodes
) {
    Piece::Type pieces[2] = { queue[node.state.next], node.state.hold };
    if (pieces[1] == Piece::Type::NONE) {
        pieces[1] = queue[node.state.next + 1];
    }
    if (pieces[0] == pieces[1]) {
        pieces[1] = Piece::Type::NONE;
    }

    for (i32 p = 0; p < 2; ++p) {
        if (pieces[p] == Piece::Type::NONE) {
            break;
        }

        generator.generate(node.state.board, pieces[p]);

        for (auto& placement : generator.locks) {
            Node::Data child = node;
            child.lock = child.state.advance(placement, queue);
            Evaluation::evaluate(child, node, placement, queue, w);
            layer.add(child);
        }

        nodes += u32(generator.locks.size());
    }
};

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
) {
    old_layer.sort();

    for (auto& node : old_layer.data) {
        Beam::expand(queue, node, new_layer, generator, w, nodes);
    }

    i32 index = -1;
    bool same_index = true;

    for (auto& node : new_layer.data) {
        candidates[node.index].visit += 1;

        if (!pc && node.state.board.is_perfect()) {
            pc = true;
        }

        if (index == -1) {
            index = node.index;
            continue;
        }

        if (index != node.index) {
            same_index = false;
        }
    }

    old_layer.clear();

    if (same_index) {
        return index;
    }

    return -1;
};

void init_candidates(
    std::vector<Candidate>& candidates,
    std::vector<Piece::Type>& queue,
    Node::Data& root,
    Layer& layer,
    Move::Generator& generator,
    Evaluation::Weight& w
) {
    Piece::Type pieces[2] = { queue[0], root.state.hold };
    if (root.state.hold == Piece::Type::NONE) {
        pieces[1] = queue[1];
    }
    if (pieces[0] == pieces[1]) {
        pieces[1] = Piece::Type::NONE;
    }

    for (i32 p = 0; p < 2; ++p) {
        if (pieces[p] == Piece::Type::NONE) {
            continue;
        }

        generator.generate(root.state.board, pieces[p]);
        for (i32 i = 0; i < i32(generator.locks.size()); ++i) {
            Candidate candidate = Candidate();
            candidate.placement = generator.locks[i];

            Node::Data child = Node::Data();
            child.state = root.state;
            child.index = i32(candidates.size());
            child.lock = child.state.advance(candidate.placement, queue);
            Evaluation::evaluate(child, root, generator.locks[i], queue, w);

            candidate.spike = Beam::get_candidate_spike(child.state, child.lock);
            candidate.visit = 0;

            candidates.push_back(candidate);
            layer.data.push_back(child);
        }
    }

    std::sort(
        layer.data.begin(),
        layer.data.end(),
        [&] (const Node::Data& a, const Node::Data& b) {
            return b < a;
        }
    );
};

std::vector<Piece::Type> get_queue_monte_carlo(Bag bag)
{
    std::vector<Piece::Type> result;

    for (i32 i = 0; i < 7; ++i) {
        if (bag[i]) {
            result.push_back(Piece::Type(i));
        }
    }

    Beam::shuffle_queue(result);

    return result;
};

u32 get_attack(State& state, Lock& lock)
{
    if (lock.clear == 0) {
        return 0;
    }

    u32 result = 0;

    if (lock.pc) {
        return 10;
    }
    
    if (lock.tspin) {
        result = lock.clear * 2;
    }
    else {
        if (lock.clear == 4) {
            result = 4;
        }
        else {
            result = lock.clear - 1;
        }
    }

    if (state.b2b > 1 && (lock.clear == 4 || lock.tspin)) {
        result == 1;
    }

    result += Evaluation::REN_LUT[std::min(state.ren, 12)];

    return result;
};

i32 get_candidate_spike(State& state, Lock& lock)
{
    return lock.clear + Beam::get_attack(state, lock);
};

bool is_queue_valid(std::vector<Piece::Type>& queue, Bag bag)
{
    for (auto p : queue) {
        if (!bag.data[static_cast<u8>(p)]) {
            return false;
        }

        bag.update(p);
    }

    return true;
};

};