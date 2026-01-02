#include "board.h"

u64& Board::operator [] (usize index)
{
    assert(index < 10);

    return this->data[index];
};

bool Board::operator == (Board& other)
{
    for (i32 i = 0; i < 10; ++i) {
        if (this->data[i] != other.data[i]) {
            return false;
        }
    }

    return true;
};

bool Board::operator != (Board& other)
{
    return !(*this == other);
};

std::array<i32, 10> Board::get_heights()
{
    std::array<i32, 10> result;

    for (i32 i = 0; i < 10; ++i) {
        result[i] = 64 - std::countl_zero(this->data[i]);
    }

    return result;
};

i32 Board::get_count()
{
    i32 result = 0;

    for (i32 i = 0; i < 10; ++i) {
        result += std::popcount(this->data[i]);
    }

    return result;
};

bool Board::is_empty()
{
    for (i32 i = 0; i < 10; ++i) {
        if (this->data[i] != 0) {
            return false;
        }
    }

    return true;
};

bool Board::is_occupied(i8 x, i8 y)
{
    if (x < 0 || x > 9 || y < 0 || y > 39) {
        return true;
    }

    return (this->data[x] >> y) & 1;
};

i32 Board::clear_lines()
{
    u64 mask = this->data[0];

    for (i32 i = 1; i < 10; ++i) {
        mask &= this->data[i];
    }
    
    if (mask == 0) {
        return 0;
    }

#ifdef PEXT
    for (i32 i = 0; i < 10; ++i) {
        this->cols[i] = _pext_u64(this->cols[i], ~mask);
    }
#else
    i32 shift = std::countr_zero(mask);

    mask = mask >> shift;

    for (i32 i = 0; i < 10; ++i) {
        u64 lo = this->data[i] & ((1ULL << shift) - 1);
        u64 hi = this->data[i] >> shift;

        switch (mask)
        {
        case 0b0001:
            hi = hi >> 1;
            break;
        case 0b0011:
            hi = hi >> 2;
            break;
        case 0b0111:
            hi = hi >> 3;
            break;
        case 0b1111:
            hi = hi >> 4;
            break;
        case 0b0101:
            hi = ((hi >> 1) & 0b0001) | ((hi >> 3) << 1);
            break;
        case 0b1001:
            hi = ((hi >> 1) & 0b0011) | ((hi >> 4) << 2);
            break;
        case 0b1011:
            hi = ((hi >> 2) & 0b0001) | ((hi >> 4) << 1);
            break;
        case 0b1101:
            hi = ((hi >> 1) & 0b0001) | ((hi >> 4) << 1);
            break;
        }

        this->data[i] = lo | (hi << shift);
    }
#endif

    return std::popcount(mask);
};

void Board::print()
{
    for (i8 y = 0; y < 25; y++) {
        for (i8 x = 0; x < 10; x++) {
            if (this->is_occupied(x, 24 - y)) {
                std::cout << '#';
            }
            else {
                std::cout << '.';
            }
        }

        std::cout << "\n";
    }
    
    std::cout << std::endl;
};