#include "path.h"

namespace path
{

path::Node::Node()
{
    this->position = move::Placement();
    this->queue.clear();
    this->queue.reserve(32);
    this->time = 0;
};

bool path::Node::operator <= (const path::Node& other)
{
    if (this->time != other.time) {
        return this->time >= other.time;
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
        return down_a >= down_b;
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

    return air_time_a <= air_time_b;
};

bool path::Node::move_right(Board& board)
{
    auto moved = this->position;

    moved.x += 1;

    if (!moved.is_colliding(board)) {
        this->position = moved;
        return true;
    }

    return false;
};

bool path::Node::move_left(Board& board)
{
    auto moved = this->position;

    moved.x -= 1;

    if (!moved.is_colliding(board)) {
        this->position = moved;
        return true;
    }

    return false;
};

bool path::Node::move_cw(Board& board)
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

bool path::Node::move_ccw(Board& board)
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

bool path::Node::move_rotate(Board& board, piece::Rotation r)
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

void path::Node::move_down(Board& board)
{
    auto moved = this->position;

    while (true)
    {
        moved.y -= 1;

        if (moved.is_colliding(board)) {
            break;
        }
    }

    this->position.y = moved.y + 1;
};

path::Map::Map()
{
    for (auto r = 0; r < 4; ++r) {
        for (auto x = 0; x < 10; ++x) {
            for (auto y = 0; y < 40; ++y) {
                this->data[r][x][y] = path::Node();
            }
        }
    }
};

bool path::Map::add(const path::Node& node)
{
    auto& get = this->data[static_cast<u8>(node.position.r)][node.position.x][node.position.y];

    if (get.queue.empty() || get <= node) {
        get = node;
        return true;
    }

    return false;
};

std::vector<Input> find(Board board, move::Placement target)
{
    target.normalize();

    auto map = Map();
    auto node = path::Node();
    auto best = path::Node();

    node.position = move::Placement(
        i8(4),
        i8(19),
        piece::Rotation::UP,
        target.type
    );

    if (node.position.is_colliding(board)) {
        node.position.y += 1;

        if (node.position.is_colliding(board)) {
            return { path::Input::DROP };
        }
    }

    path::expand(node, board, map, target, best);

    if (best.queue.size() == 1) {
        best.queue.insert(best.queue.begin(), path::Input::NONE);
    }

    return best.queue;
};

void expand(const path::Node& node, Board& board, Map& map, const move::Placement& target, path::Node& best)
{
    // Drop
    auto drop = node;

    drop.move_down(board);

    auto lock = drop;

    lock.position.normalize();
    lock.queue.push_back(path::Input::DROP);
    lock.time += 1;

    if (lock.position == target && drop.position.y < 21) {
        if (best.queue.empty() || best <= lock) {
            best = lock;
            return;
        }
    }

    if (drop.position.y != node.position.y) {
        if (!drop.queue.empty() && drop.queue.back() == path::Input::DOWN) {
            drop.queue.push_back(path::Input::NONE);
            drop.time += 1;
        }

        drop.queue.push_back(path::Input::DOWN);
        drop.time += std::abs(node.position.y - drop.position.y) * 2 + 1;

        if (map.add(drop)) {
            path::expand(drop, board, map, target, best);
        }
    }

    bool dropped = std::find(node.queue.begin(), node.queue.end(), path::Input::DOWN) != node.queue.end();

    // Right
    auto right = node;

    if (right.move_right(board) && !(right.position.is_above_stack(board) && dropped)) {
        if (!right.queue.empty() && right.queue.back() == path::Input::RIGHT) {
            right.queue.push_back(path::Input::NONE);
            right.time += 1;
        }

        right.queue.push_back(path::Input::RIGHT);
        right.time += 1;

        if (map.add(right)) {
            path::expand(right, board, map, target, best);
        }
    }

    // Left
    auto left = node;

    if (left.move_left(board) && !(left.position.is_above_stack(board) && dropped)) {
        if (!left.queue.empty() && left.queue.back() == path::Input::LEFT) {
            left.queue.push_back(path::Input::NONE);
            left.time += 1;
        }

        left.queue.push_back(path::Input::LEFT);
        left.time += 1;

        if (map.add(left)) {
            path::expand(left, board, map, target, best);
        }
    }

    if (node.position.type == piece::Type::O) {
        return;
    }

    // Clockwise
    auto cw = node;

    if (cw.move_cw(board) && !(cw.position.is_above_stack(board) && dropped)) {
        if (!cw.queue.empty() && cw.queue.back() == path::Input::CW) {
            cw.queue.push_back(path::Input::NONE);
            cw.time += 1;
        }

        cw.queue.push_back(path::Input::CW);
        cw.time += 1;

        if (map.add(cw)) {
            path::expand(cw, board, map, target, best);
        }
    }

    // Counter clockwise
    auto ccw = node;

    if (ccw.move_ccw(board) && !(ccw.position.is_above_stack(board) && dropped)) {
        if (!ccw.queue.empty() && ccw.queue.back() == path::Input::CCW) {
            ccw.queue.push_back(path::Input::NONE);
            ccw.time += 1;
        }

        ccw.queue.push_back(path::Input::CCW);
        ccw.time += 1;

        if (map.add(ccw)) {
            path::expand(ccw, board, map, target, best);
        }
    }
};

};