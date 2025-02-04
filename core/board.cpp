#include "board.h"

u64& Board::operator [] (i32 index)
{
    assert(index > -1 && index < 10);
    return this->cols[index];
};

bool Board::operator == (Board& other)
{
    for (i32 i = 0; i < 10; ++i) {
        if (this->cols[i] != other.cols[i]) {
            return false;
        }
    }

    return true;
};

bool Board::operator != (Board& other)
{
    return !(*this == other);
};

void Board::get_heights(i32 heights[10])
{
    for (i32 i = 0; i < 10; ++i) {
        heights[i] = 64 - std::countl_zero(this->cols[i]);
    }
};

u64 Board::get_clear_mask()
{
    u64 result = this->cols[0];

    for (i32 i = 1; i < 10; ++i) {
        result &= this->cols[i];
    }

    return result;
};

i32 Board::get_count()
{
    i32 result = 0;

    for (i32 i = 0; i < 10; ++i) {
        result += std::popcount(this->cols[i]);
    }

    return result;
};

bool Board::is_empty()
{
    for (i32 i = 0; i < 10; ++i) {
        if (this->cols[i] != 0) {
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

    return (this->cols[x] >> y) & 1;
};

i32 Board::clear()
{
    u64 mask = this->get_clear_mask();
    
    if (mask == 0) {
        return 0;
    }

#ifdef PEXT
    for (int i = 0; i < 10; ++i) {
        this->cols[i] = _pext_u64(this->cols[i], ~mask);
    }
#else
    int mask_tzcnt = std::countr_zero(mask);
    mask = mask >> mask_tzcnt;

    for (int i = 0; i < 10; ++i) {
        u64 low_part = this->cols[i] & ((1ULL << mask_tzcnt) - 1);
        u64 high_part = this->cols[i] >> mask_tzcnt;

        switch (mask)
        {
        case 0b0001:
            high_part = high_part >> 1;
            break;
        case 0b0011:
            high_part = high_part >> 2;
            break;
        case 0b0111:
            high_part = high_part >> 3;
            break;
        case 0b1111:
            high_part = high_part >> 4;
            break;
        case 0b0101:
            high_part = ((high_part >> 1) & 0b0001) | ((high_part >> 3) << 1);
            break;
        case 0b1001:
            high_part = ((high_part >> 1) & 0b0011) | ((high_part >> 4) << 2);
            break;
        case 0b1011:
            high_part = ((high_part >> 2) & 0b0001) | ((high_part >> 4) << 1);
            break;
        case 0b1101:
            high_part = ((high_part >> 1) & 0b0001) | ((high_part >> 4) << 1);
            break;
        }

        this->cols[i] = low_part | (high_part << mask_tzcnt);
    }
#endif

    return std::popcount(mask);
};

void Board::print()
{
    using namespace std;

    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 10; x++) {
            char cell = '.';

            if (this->is_occupied(int8_t(x), int8_t(24 - y))) {
                cell = '#';
            }

            cout << cell;
        }

        cout << "\n";
    }
    
    cout << endl;
};