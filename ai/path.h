#pragma once

#include "beam.h"

namespace path
{

enum class Input
{
    RIGHT,
    LEFT,
    CW,
    CCW,
    DOWN,
    DROP,
    NONE
};

typedef std::vector<Input> Queue;

struct Node
{
public:
    move::Placement position;
    std::vector<Input> queue;
    size_t time;
public:
    Node();
public:
    bool operator <= (const path::Node& other);
public:
    bool move_right(Board& board);
    bool move_left(Board& board);
    bool move_cw(Board& board);
    bool move_ccw(Board& board);
    bool move_rotate(Board& board, piece::Rotation r);
    void move_down(Board& board);
};

class Map
{
public:
    path::Node data[4][10][40];
public:
    Map();
public:
    bool add(const path::Node& node);
};

std::vector<Input> find(Board board, move::Placement target);

void expand(const path::Node& node, Board& board, Map& map, const move::Placement& target, path::Node& best);

constexpr std::string input_to_str(Input input)
{
    switch (input)
    {
    case Input::NONE:
        return " ";
        break;
    case Input::RIGHT:
        return "RIGHT";
        break;
    case Input::LEFT:
        return "LEFT";
        break;
    case Input::CW:
        return "CW";
        break;
    case Input::CCW:
        return "CCW";
        break;
    case Input::DOWN:
        return "DOWN";
        break;
    case Input::DROP:
        return "DROP";
        break;
    default:
        break;
    }

    return " ";
};

inline void print(std::vector<Input> inputs)
{
    for (auto& i : inputs) {
        std::cout << input_to_str(i) << " ";
    }

    printf("\n");
};

};