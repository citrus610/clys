#include "layer.h"

Layer::Layer(usize width)
{
    this->width = width;
    this->map.reserve(1 << 12);
    this->data.reserve(width);
};

void Layer::clear()
{
    this->data.clear();
    this->map.clear();
};

void Layer::add(node::Data& node)
{
    auto hash = node.state.get_hash();
    auto find = this->map.find(hash);

    if (find != this->map.end()) {
        if (node.score.reward < find->second) {
            return;
        }

        find->second = node.score.reward;
    }
    else {
        this->map.insert({ hash, node.score.reward });
    }

    if (this->data.size() < this->width) {
        this->data.push_back(node);

        if (this->data.size() == this->width) {
            std::make_heap(
                this->data.begin(),
                this->data.end(),
                [&] (const node::Data& a, const node::Data& b) {
                    return b < a;
                }
            );
        }

        return;
    }
    
    if (this->data.front() < node) {
        std::pop_heap(
            this->data.begin(),
            this->data.end(),
            [&] (const node::Data& a, const node::Data& b) {
                return b < a;
            }
        );

        this->data.back() = node;

        std::push_heap(
            this->data.begin(),
            this->data.end(),
            [&] (const node::Data& a, const node::Data& b) {
                return b < a;
            }
        );
    }
};

void Layer::sort()
{
    if (this->data.size() < this->width) {
        std::sort(
            this->data.begin(),
            this->data.end(),
            [&] (node::Data& a, node::Data& b) {
                return b < a;
            }
        );

        return;
    }

    std::sort_heap(
        this->data.begin(),
        this->data.end(),
        [&] (node::Data& a, node::Data& b) {
            return b < a;
        }
    );
};