#include "piece.h"
#include "board.h"

namespace Piece
{

bool Data::operator == (Data& other)
{
    return
        this->x == other.x &&
        this->y == other.y &&
        this->r == other.r &&
        this->type == other.type;
};

bool Data::operator == (const Data& other)
{
    return
        this->x == other.x &&
        this->y == other.y &&
        this->r == other.r &&
        this->type == other.type;
};

bool Data::move_right(Board& board)
{
    if (!board.is_colliding(this->x + 1, this->y, this->type, this->r)) {
        ++this->x;
        return true;
    }
    return false;
};

bool Data::move_left(Board& board)
{
    if (!board.is_colliding(this->x - 1, this->y, this->type, this->r)) {
        --this->x;
        return true;
    }
    return false;
};

bool Data::move_cw(Board& board)
{
    Rotation new_r = this->r;
    switch (this->r)
    {
    case Rotation::UP:
        new_r = Rotation::RIGHT;
        break;
    case Rotation::RIGHT:
        new_r = Rotation::DOWN;
        break;
    case Rotation::DOWN:
        new_r = Rotation::LEFT;
        break;
    case Rotation::LEFT:
        new_r = Rotation::UP;
        break;
    }

    return this->move_rotate(board, new_r);
};

bool Data::move_ccw(Board& board)
{
    Rotation new_r = this->r;
    switch (this->r)
    {
    case Rotation::UP:
        new_r = Rotation::LEFT;
        break;
    case Rotation::LEFT:
        new_r = Rotation::DOWN;
        break;
    case Rotation::DOWN:
        new_r = Rotation::RIGHT;
        break;
    case Rotation::RIGHT:
        new_r = Rotation::UP;
        break;
    }

    return this->move_rotate(board, new_r);
};

bool Data::move_rotate(Board& board, Rotation new_r)
{
    i8 srs_index = std::min(static_cast<i8>(this->type), i8(1));

    for (i32 i = 0; i < 5; ++i) {
        i8 offset_x = get_offset_srs_x(srs_index, this->r, i) - get_offset_srs_x(srs_index, new_r, i);
        i8 offset_y = get_offset_srs_y(srs_index, this->r, i) - get_offset_srs_y(srs_index, new_r, i);
        if (!board.is_colliding(this->x + offset_x, this->y + offset_y, this->type, new_r)) {
            this->x += offset_x;
            this->y += offset_y;
            this->r = new_r;
            return true;
        }
    }

    return false;
};

bool Data::move_down(Board& board)
{
    if (!board.is_colliding(this->x, this->y - 1, this->type, this->r)) {
        --this->y;
        return true;
    }
    return false;
};

void Data::move_drop(Board& board)
{
    this->y -= board.get_drop_distance(*this);
};

void Data::normalize()
{
    switch (this->type)
    {
    case Type::I:
        switch (this->r)
        {
        case Rotation::UP:
            break;
        case Rotation::RIGHT:
            break;
        case Rotation::DOWN:
            this->r = Rotation::UP;
            --this->x;
            break;
        case Rotation::LEFT:
            this->r = Rotation::RIGHT;
            ++this->y;
            break;
        default:
            break;
        }
        break;
    case Type::S:
        switch (r)
        {
        case Rotation::UP:
            break;
        case Rotation::RIGHT:
            break;
        case Rotation::DOWN:
            this->r = Rotation::UP;
            --this->y;
            break;
        case Rotation::LEFT:
            this->r = Rotation::RIGHT;
            --this->x;
            break;
        default:
            break;
        }
        break;
    case Type::Z:
        switch (r)
        {
        case Rotation::UP:
            break;
        case Rotation::RIGHT:
            break;
        case Rotation::DOWN:
            this->r = Rotation::UP;
            --this->y;
            break;
        case Rotation::LEFT:
            this->r = Rotation::RIGHT;
            --this->x;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
};

Data Data::get_normalize()
{
    Data copy = *this;
    copy.normalize();
    return copy;
};

}