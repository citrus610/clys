#include "beam.h"

namespace beam
{

// Expands a node and returns the number of children
u32 expand(
    std::vector<piece::Type>& queue,
    node::Data& node,
    std::function<void(node::Data&, move::Placement)> callback
)
{
    u32 nodes = 0;

    piece::Type pieces[2] = { queue[node.state.next], node.state.hold };

    if (pieces[1] == piece::Type::NONE) {
        pieces[1] = queue[node.state.next + 1];
    }

    if (pieces[0] == pieces[1]) {
        pieces[1] = piece::Type::NONE;
    }

    for (i32 p = 0; p < 2; ++p) {
        if (pieces[p] == piece::Type::NONE) {
            break;
        }

        auto locks = move::generate(node.state.board, pieces[p]);

        for (auto& placement : locks) {
            auto child = node;
            child.lock = child.state.advance(placement, queue);
            callback(child, placement);
        }

        nodes += u32(locks.size());
    }

    return nodes;
};

// Does 1 thinking cycle of the beam search
// Expands the top-k nodes of the previous layer and add their children to the next layer
// Returns a candidate's index if all children come from the same ancestor, else return -1
i32 think(
    std::vector<piece::Type>& queue,
    std::vector<Candidate>& candidates,
    Layer& parents,
    Layer& children,
    const eval::Weight& w,
    u32& nodes
)
{
    // Sorts the parents layer
    parents.sort();

    // Expands each parent to the next layer
    for (auto& node : parents.data) {
        nodes += beam::expand(queue, node, [&] (node::Data& child, move::Placement placement) {
            eval::evaluate(child, node, placement, queue, w);
            children.add(child);
        });
    }

    i32 index = -1;
    bool same_index = true;

    // Updates candidate's visit count
    for (auto& node : children.data) {
        candidates[node.index].visit += 1;

        if (index == -1) {
            index = node.index;
            continue;
        }

        if (index != node.index) {
            same_index = false;
        }
    }

    // Clears the parents layer
    parents.clear();

    if (same_index) {
        return index;
    }

    return -1;
};

// Monte Carlo Beam Search
// This function implemented the beam search algorithm
// To search past the input queue, we create multiple possible future queues using Monte Carlo method and beam search those queues
// The search results are then concatenated to get the average evaluation for each candidate
// The candidate with the highest visit count will be chosen
// Ideally, we would want to search as many branches as possible
// However, searching multiple queues is very expensive so we only search a small number of branches here
Result search(
    State state,
    Lock lock,
    std::vector<piece::Type> queue,
    eval::Weight w,
    Configs configs,
    std::atomic_flag& running
)
{
    auto result = Result();

    // Checks queue
    if (queue.size() < 2 || !beam::is_queue_valid(queue, state.bag)) {
        return result;
    }

    // Creates root
    auto root = node::Data {
        .state = state,
        .score = { 0, 0 },
        .lock = lock,
        .index = -1
    };

    root.state.next = 0;

    // Creates stack
    std::array<Layer, 2> layers = {
        Layer(configs.width),
        Layer(configs.width)
    };

    // Initializes candidates
    result.nodes += beam::expand(
        queue,
        root,
        [&] (node::Data& child, move::Placement placement) {
            auto candidate = beam::Candidate();

            candidate.placement = placement;
            candidate.visit = 0;

            child.index = i32(result.candidates.size());
            eval::evaluate(child, root, placement, queue, w);

            result.candidates.push_back(candidate);
            layers[0].data.push_back(child);
        }
    );

    // If there aren't any candidates, stops searching
    if (result.candidates.empty()) {
        return result;
    }

    // Searches the input queue
    result.depth = 1;

    for (size_t i = 0; i < queue.size() - 1 - (root.state.hold == piece::Type::NONE); ++i) {
        i32 index = beam::think(
            queue,
            result.candidates,
            layers[(result.depth - 1) & 1],
            layers[result.depth & 1],
            w,
            result.nodes
        );

        result.depth += 1;

        // If all children come from the same ancestor, we don't have to keep searching
        if (index != -1) {
            result.candidates[index].visit += configs.depth * configs.width;
            result.nodes += u32(configs.depth * configs.width * 68);
            result.depth = configs.depth;
            return result;
        }
    }

    // Normalizes visit counts
    for (auto& c : result.candidates) {
        c.visit *= configs.branch;
    }

    // Creates random queues
    auto bag = root.state.bag;

    for (auto& p : queue) {
        bag.update(p);
    }

    std::vector<std::vector<piece::Type>> future_queues;

    for (size_t i = 0; i < configs.branch; ++i) {
        auto random_queue = beam::get_queue_random(bag, configs.depth);
        random_queue.insert(random_queue.begin(), queue.begin(), queue.end());
        future_queues.push_back(random_queue);
    }

    // Initializes future layers stack
    std::vector<std::array<Layer, 2>> future_layers;
        
    for (size_t i = 0; i < configs.branch; ++i) {
        future_layers.push_back({
            Layer(configs.width),
            Layer(configs.width)
        });

        future_layers.back()[(result.depth - 1) & 1].data = layers[(result.depth - 1) & 1].data;
    }

    std::vector<i32> future_indices;

    for (size_t i = 0; i < configs.branch; ++i) {
        future_indices.push_back(-1);
    }

    // Search future queues
    while (running.test() && result.depth < configs.depth)
    {
        // For every branch, we do 1 cycle of beam search
        for (size_t i = 0; i < configs.branch; ++i) {
            if (future_indices[i] != -1) {
                // If all the children of this branch come from the same ancestor, we don't have to expand nodes manually
                // Instead, we can just increase that ancestor's visit count accordingly
                result.candidates[future_indices[i]].visit += configs.width;
            }
            else {
                future_indices[i] = beam::think(
                    future_queues[i],
                    result.candidates,
                    future_layers[i][(result.depth - 1) & 1],
                    future_layers[i][result.depth & 1],
                    w,
                    result.nodes
                );
            }
        }

        result.depth += 1;
    }

    return result;
};

// Returns a possible random queue according to the 7-bag system
std::vector<piece::Type> get_queue_random(Bag bag, size_t count)
{
    std::vector<piece::Type> result;

    while (result.size() < count)
    {
        std::vector<piece::Type> next;

        // Adds the remaining pieces in the bag to the next queue
        for (i32 i = 0; i < 7; ++i) {
            if (bag.get(piece::Type(i))) {
                next.push_back(piece::Type(i));
            }
        }

        // Shuffle the next queue
        for (size_t i = 0; i < next.size(); ++i) {
            size_t k = size_t(rand()) % next.size();
            std::swap(next[i], next[k]);
        }

        // Inserts the next queue into the result
        result.insert(result.end(), next.begin(), next.end());

        // Fills the bag
        bag = Bag();
    }

    return result;
};

// Checks if the input queue is valid according to the 7-bag system
bool is_queue_valid(const std::vector<piece::Type>& queue, Bag bag)
{
    for (auto p : queue) {
        if (!bag.get(p)) {
            return false;
        }

        bag.update(p);
    }

    return true;
};

};