#pragma once

#include "ttable.h"

class Layer
{
public:
    Transposition::Table ttable;
    std::vector<Node::Data> data;
    size_t width;
public:
    Layer();
    ~Layer();
public:
    void init(size_t width);
    void clear();
    void add(Node::Data& node);
    void sort();
};