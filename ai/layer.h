#pragma once

#include "node.h"

class Layer
{
public:
    std::unordered_map<u64, i32> map;
    std::vector<node::Data> data;
    size_t width;
public:
    Layer(size_t width);
public:
    void clear();
    void add(node::Data& node);
    void sort();
};