#pragma once

#include "node.h"

class Layer
{
public:
    std::unordered_map<u64, i32> map;
    std::vector<node::Data> data;
    usize width;
public:
    Layer(usize width);
public:
    void clear();
    void add(node::Data& node);
    void sort();
};