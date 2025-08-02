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
    WAIT,
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
    bool operator < (const path::Node& other) const;
    bool operator == (const path::Node& other) const;
public:
    bool attempt(Board& board, Input move);
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
    path::Node data[10][40][4];
public:
    Map();
public:
    bool get(move::Placement placement, Node& node);
    bool add(move::Placement placement, Node& node);
public:
    void clear();
};

Queue find(Board board, move::Placement destination, bool force_20);

void expand(Board board, Node& node, std::vector<Node>& queue, Map& map_queue);

void lock(Board board, Node& node, std::vector<Node>& locks, Map& map_locks);

void add(Node& node, std::vector<Node>& queue, Map& map_queue);

int index(Node& node, std::vector<Node>& queue);

};