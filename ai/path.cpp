#include "path.h"

namespace Path
{

Node::Node()
{
    this->placement = Piece::Data();
    this->move.reserve(32);
    this->move.clear();
    this->time = 0;
};

bool Node::attempt(Board& board, Move move)
{
    bool success = false;
    Piece::Data previous = this->placement;

    switch (move)
    {
    case Move::RIGHT:
        success = this->placement.move_right(board);
        break;
    case Move::LEFT:
        success = this->placement.move_left(board);
        break;
    case Move::CW:
        success = this->placement.move_cw(board);
        break;
    case Move::CCW:
        success = this->placement.move_ccw(board);
        break;
    case Move::DOWN:
        this->placement.move_drop(board);
        success = this->placement.y <= previous.y;
        if (success) this->time += 2;
        break;
    default:
        break;
    }

    if (!success) {
        return false;
    }

    if (!this->move.empty() && this->move.back() == move) {
        this->time += 1;
    }

    this->move.push_back(move);
    this->time += 1;

    if (move == Move::DOWN) {
        this->time += std::abs(previous.y - this->placement.y) * 2;
    }

    return true;
};

bool Node::operator < (Node& other)
{
    if (std::count(this->move.begin(), this->move.end(), Move::DOWN) == std::count(other.move.begin(), other.move.end(), Move::DOWN)) {
        return this->time < other.time;
    }
    return std::count(this->move.begin(), this->move.end(), Move::DOWN) < std::count(other.move.begin(), other.move.end(), Move::DOWN);
};

bool Node::operator > (Node& other)
{
    if (std::count(this->move.begin(), this->move.end(), Move::DOWN) == std::count(other.move.begin(), other.move.end(), Move::DOWN)) {
        return this->time > other.time;
    }
    return std::count(this->move.begin(), this->move.end(), Move::DOWN) > std::count(other.move.begin(), other.move.end(), Move::DOWN);
};

bool Node::operator == (Node& other)
{
    return this->time == other.time && std::count(this->move.begin(), this->move.end(), Move::DOWN) == std::count(other.move.begin(), other.move.end(), Move::DOWN);
};

Map::Map()
{
    this->clear();
};

bool Map::get(Piece::Data placement, Node& node)
{
    node = this->data[placement.x][placement.y][static_cast<u8>(placement.r)];
    return !node.move.empty();
};

bool Map::add(Piece::Data placement, Node& node)
{
    if (this->data[placement.x][placement.y][static_cast<u8>(placement.r)].move.empty()) {
        this->data[placement.x][placement.y][static_cast<u8>(placement.r)] = node;
        return true;
    }

    if (!(node > this->data[placement.x][placement.y][static_cast<u8>(placement.r)])) {
        this->data[placement.x][placement.y][static_cast<u8>(placement.r)] = node;
        return true;
    }

    return false;
};

void Map::clear()
{
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 25; ++y) {
            for (int r = 0; r < 4; ++r) {
                this->data[x][y][r].placement = Piece::Data(x, y, Piece::Type::NONE, Piece::Rotation(r));
                this->data[x][y][r].move.clear();
                this->data[x][y][r].time = 0;
            }
        }
    }
};

std::vector<Move> search(Board board, Piece::Data destination)
{
    std::vector<Move> move;
    move.clear();

    if (destination.type == Piece::Type::NONE || board.get_drop_distance(destination) != 0) {
        move.push_back(Move::DOWN);
        return move;
    }

    std::vector<Node> queue;
    std::vector<Node> locks;
    Map map_queue;
    Map map_locks;

    Node init = Node();

    init.placement = Piece::Data(4, 19, destination.type, Piece::Rotation::UP);
    if (board.is_colliding(init.placement)) {
        init.placement.y = 20;
        if (board.is_colliding(init.placement)) {
            move.push_back(Move::DOWN);
            return move;
        }
    }

    queue.push_back(init);
    map_queue.add(init.placement, init);

    while (!queue.empty())
    {
        Node node = queue.back();
        queue.pop_back();

        expand(board, node, queue, map_queue);
        lock(board, node, locks, map_locks);
    }

    Node final;
    map_locks.get(destination.get_normalize(), final);
    move = final.move;

    if (move.empty() || move.back() != Move::DOWN) {
        move.push_back(Move::DOWN);
    }

    return move;
};

void expand(Board board, Node& node, std::vector<Node>& queue, Map& map_queue)
{
    Node n_drop = node;
    if (n_drop.attempt(board, Move::DOWN)) {
        add(n_drop, queue, map_queue);
    }

    Node n_right = node;
    if (n_right.attempt(board, Move::RIGHT)) {
        add(n_right, queue, map_queue);
    }

    Node n_left = node;
    if (n_left.attempt(board, Move::LEFT)) {
        add(n_left, queue, map_queue);
    }

    if (node.placement.type == Piece::Type::O) {
        return;
    }

    Node n_cw = node;
    if (n_cw.attempt(board, Move::CW)) {
        add(n_cw, queue, map_queue);
    }

    Node n_ccw = node;
    if (n_ccw.attempt(board, Move::CCW)) {
        add(n_ccw, queue, map_queue);
    }
};

void lock(Board board, Node& node, std::vector<Node>& locks, Map& map_locks)
{
    node.placement.move_drop(board);
    node.placement.normalize();

    if (!node.move.empty() && (node.move.back() == Move::CW || node.move.back() == Move::CCW)) {
        node.time -= 1;
    }

    if (node.move.empty() || node.move.back() != Move::DOWN) {
        node.move.push_back(Move::DOWN);
        node.time += 1;
    }

    if (!map_locks.add(node.placement, node)) {
        return;
    }

    int idx = index(node, locks);

    if (idx == -1) {
        locks.push_back(node);
    }
    else if (node < locks[idx]) {
        locks[idx] = node;
    }
};

void add(Node& node, std::vector<Node>& queue, Map& map_queue)
{
    if (!map_queue.add(node.placement, node)) {
        return;
    }

    int idx = index(node, queue);

    if (idx == -1) {
        queue.push_back(node);
    }
    else if (node < queue[idx]) {
        queue[idx] = node;
    }
    else if (node == queue[idx]) {
        queue.push_back(node);
    }
};

int index(Node& node, std::vector<Node>& queue)
{
    for (int i = 0; i < int(queue.size()); ++i) {
        if (node.placement == queue[i].placement) {
            return i;
        }
    }
    return -1;
};

};