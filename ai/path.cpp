#include "path.h"

namespace path
{

Node::Node()
{
    this->position = move::Placement();
    this->queue.clear();
    this->queue.reserve(32);
    this->time = 0;
};

bool Node::attempt(Board& board, Input move)
{
    bool success = false;
    auto previous = this->position;

    switch (move)
    {
    case Input::RIGHT:
        success = this->move_right(board);
        break;
    case Input::LEFT:
        success = this->move_left(board);
        break;
    case Input::CW:
        success = this->move_cw(board);
        break;
    case Input::CCW:
        success = this->move_ccw(board);
        break;
    case Input::DOWN:
        this->move_down(board);
        success = this->position.y <= previous.y;
        if (success) this->time += 2;
        break;
    default:
        break;
    }

    if (!success) {
        return false;
    }

    if (!this->queue.empty() && this->queue.back() == move) {
        this->queue.push_back(Input::NONE);
        this->time += 1;
    }

    this->queue.push_back(move);
    this->time += 1;

    if (move == Input::DOWN) {
        this->time += std::abs(previous.y - this->position.y) * 2;
    }

    return true;
};

bool Node::operator < (const Node& other) const
{
    if (this->time != other.time) {
        return this->time > other.time;
    }

    size_t down_a = 0;
    size_t down_b = 0;

    for (auto& i : this->queue) {
        if (i == Input::DOWN) {
            down_a += 1;
        }
    }

    for (auto& i : other.queue) {
        if (i == Input::DOWN) {
            down_b += 1;
        }
    }

    if (down_a != down_b) {
        return down_a > down_b;
    }

    size_t air_time_a = 0;
    size_t air_time_b = 0;

    for (auto& i : this->queue) {
        if (i == Input::DOWN) {
            break;
        }

        air_time_a += 1;
    }

    for (auto& i : other.queue) {
        if (i == Input::DOWN) {
            break;
        }

        air_time_b += 1;
    }

    return air_time_a < air_time_b;
};

bool Node::operator == (const Node& other) const
{
    size_t down_a = 0;
    size_t down_b = 0;

    for (auto& i : this->queue) {
        if (i == Input::DOWN) {
            down_a += 1;
        }
    }

    for (auto& i : other.queue) {
        if (i == Input::DOWN) {
            down_b += 1;
        }
    }

    size_t air_time_a = 0;
    size_t air_time_b = 0;

    for (auto& i : this->queue) {
        if (i == Input::DOWN) {
            break;
        }

        air_time_a += 1;
    }

    for (auto& i : other.queue) {
        if (i == Input::DOWN) {
            break;
        }

        air_time_b += 1;
    }

    return this->time == other.time && down_a == down_b && air_time_a == air_time_b;
};

bool Node::move_right(Board& board)
{
    auto moved = this->position;

    moved.x += 1;

    if (!moved.is_colliding(board)) {
        this->position = moved;
        return true;
    }

    return false;
};

bool Node::move_left(Board& board)
{
    auto moved = this->position;

    moved.x -= 1;

    if (!moved.is_colliding(board)) {
        this->position = moved;
        return true;
    }

    return false;
};

bool Node::move_cw(Board& board)
{
    piece::Rotation r;

    switch (this->position.r)
    {
    case piece::Rotation::UP:
        r = piece::Rotation::RIGHT;
        break;
    case piece::Rotation::RIGHT:
        r = piece::Rotation::DOWN;
        break;
    case piece::Rotation::DOWN:
        r = piece::Rotation::LEFT;
        break;
    case piece::Rotation::LEFT:
        r = piece::Rotation::UP;
        break;
    }

    return this->move_rotate(board, r);
};

bool Node::move_ccw(Board& board)
{
    piece::Rotation r;

    switch (this->position.r)
    {
    case piece::Rotation::UP:
        r = piece::Rotation::LEFT;
        break;
    case piece::Rotation::LEFT:
        r = piece::Rotation::DOWN;
        break;
    case piece::Rotation::DOWN:
        r = piece::Rotation::RIGHT;
        break;
    case piece::Rotation::RIGHT:
        r = piece::Rotation::UP;
        break;
    }

    return this->move_rotate(board, r);
};

bool Node::move_rotate(Board& board, piece::Rotation r)
{
    i8 srs_index = this->position.type != piece::Type::I;

    for (i32 i = 0; i < 5; ++i) {
        i8 offset_x = move::get_srs_x(srs_index, this->position.r, i) - move::get_srs_x(srs_index, r, i);
        i8 offset_y = move::get_srs_y(srs_index, this->position.r, i) - move::get_srs_y(srs_index, r, i);

        auto moved = this->position;

        moved.x += offset_x;
        moved.y += offset_y;
        moved.r = r;

        if (!moved.is_colliding(board)) {
            this->position = moved;
            return true;
        }
    }

    return false;
};

void Node::move_down(Board& board)
{
    while (true)
    {
        auto moved = this->position;
        moved.y -= 1;

        if (moved.is_colliding(board)) {
            break;
        }

        this->position.y -= 1;
    }
};

Map::Map()
{
    this->clear();
};

bool Map::get(move::Placement placement, Node& node)
{
    node = this->data[placement.x][placement.y][static_cast<u8>(placement.r)];
    return !node.queue.empty();
};

bool Map::add(move::Placement placement, Node& node)
{
    if (this->data[placement.x][placement.y][static_cast<u8>(placement.r)].queue.empty()) {
        this->data[placement.x][placement.y][static_cast<u8>(placement.r)] = node;
        return true;
    }

    if (!(node < this->data[placement.x][placement.y][static_cast<u8>(placement.r)])) {
        this->data[placement.x][placement.y][static_cast<u8>(placement.r)] = node;
        return true;
    }

    return false;
};

void Map::clear()
{
    for (i32 x = 0; x < 10; ++x) {
        for (i32 y = 0; y < 40; ++y) {
            for (i32 r = 0; r < 4; ++r) {
                this->data[x][y][r].position = move::Placement(x, y, piece::Rotation(r), piece::Type::NONE);
                this->data[x][y][r].queue.clear();
                this->data[x][y][r].time = 0;
            }
        }
    }
};

Queue find(Board board, move::Placement destination, bool force_20)
{
    std::vector<Input> move;
    move.clear();

    if (destination.type == piece::Type::NONE) {
        move.push_back(Input::NONE);
        move.push_back(Input::DROP);
        return move;
    }

    std::vector<Node> queue;
    std::vector<Node> locks;
    Map map_queue;
    Map map_locks;

    Node init = Node();

    init.position = move::Placement(4, 19, piece::Rotation::UP, destination.type);

    if (init.position.is_colliding(board)) {
        init.position.y = 20;

        if (init.position.is_colliding(board)) {
            move.push_back(Input::NONE);
            move.push_back(Input::DROP);
            return move;
        }
    }

    if (force_20) {
        init.position.y = 20;
    }

    queue.push_back(init);
    map_queue.add(init.position, init);

    while (!queue.empty())
    {
        Node node = queue.back();
        queue.pop_back();

        expand(board, node, queue, map_queue);
        lock(board, node, locks, map_locks);
    }

    Node final;
    map_locks.get(destination.get_normalize(), final);
    move = final.queue;

    if (move.empty() || move.back() != Input::DROP) {
        move.push_back(Input::DROP);
    }

    if (move.size() == 1) {
        move.insert(move.begin(), Input::NONE);
    }

    return move;
};

void expand(Board board, Node& node, std::vector<Node>& queue, Map& map_queue)
{
    Node n_drop = node;
    if (n_drop.attempt(board, Input::DOWN)) {
        add(n_drop, queue, map_queue);
    }

    Node n_right = node;
    if (n_right.attempt(board, Input::RIGHT)) {
        add(n_right, queue, map_queue);
    }

    Node n_left = node;
    if (n_left.attempt(board, Input::LEFT)) {
        add(n_left, queue, map_queue);
    }

    if (node.position.type == piece::Type::O) {
        return;
    }

    Node n_cw = node;
    if (n_cw.attempt(board, Input::CW)) {
        add(n_cw, queue, map_queue);
    }

    Node n_ccw = node;
    if (n_ccw.attempt(board, Input::CCW)) {
        add(n_ccw, queue, map_queue);
    }
};

void lock(Board board, Node& node, std::vector<Node>& locks, Map& map_locks)
{
    node.move_down(board);
    node.position.normalize();


    if (node.queue.empty() || node.queue.back() != Input::DROP) {
        node.queue.push_back(Input::DROP);
        node.time += 1;
    }

    if (!map_locks.add(node.position, node)) {
        return;
    }

    int idx = index(node, locks);

    if (idx == -1) {
        locks.push_back(node);
    }
    else if (locks[idx] < node) {
        locks[idx] = node;
    }
};

void add(Node& node, std::vector<Node>& queue, Map& map_queue)
{
    if (!map_queue.add(node.position, node)) {
        return;
    }

    int idx = index(node, queue);

    if (idx == -1) {
        queue.push_back(node);
    }
    else if (queue[idx] < node) {
        queue[idx] = node;
    }
    else if (queue[idx] == node) {
        queue.push_back(node);
    }
};

int index(Node& node, std::vector<Node>& queue)
{
    for (int i = 0; i < int(queue.size()); ++i) {
        if (node.position == queue[i].position) {
            return i;
        }
    }
    return -1;
};

};