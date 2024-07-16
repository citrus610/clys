#include "layer.h"

Layer::Layer()
{
    this->ttable = Transposition::Table();
    this->data.clear();
};

Layer::~Layer()
{

};

void Layer::init(size_t width)
{
    this->width = width;
    this->ttable.init();
    this->data.reserve(this->width);
    this->clear();
};

void Layer::clear()
{
    this->ttable.clear();
    this->data.clear();
};

void Layer::add(Node::Data& node)
{
    // Check transposition table
    uint32_t hash = this->ttable.hash(node.state);
    if (!this->ttable.add_entry(hash, node.score.acml)) {
        return;
    }

    // Push to data
    if (this->data.size() < this->width) {
        this->data.push_back(node);

        if (this->data.size() == this->width) {
            std::make_heap(this->data.begin(), this->data.end(), [&] (Node::Data& a, Node::Data& b) { return b < a; });
        }

        return;
    }
    
    if (this->data[0] < node) {
        std::pop_heap(this->data.begin(), this->data.end(), [&] (Node::Data& a, Node::Data& b) { return b < a; });
        this->data.back() = node;
        std::push_heap(this->data.begin(), this->data.end(), [&] (Node::Data& a, Node::Data& b) { return b < a; });
    }
};

void Layer::sort()
{
    if (this->data.size() < this->width) {
        std::sort(
            this->data.begin(),
            this->data.end(),
            [&] (Node::Data& a, Node::Data& b) { return b < a; }
        );

        return;
    }

    std::sort_heap(
        this->data.begin(),
        this->data.end(),
        [&] (Node::Data& a, Node::Data& b) { return b < a; }
    );
};