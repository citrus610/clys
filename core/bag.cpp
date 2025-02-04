#include "bag.h"

Bag::Bag()
{
    this->data = 0b1111111;
};

bool Bag::get(const piece::Type& piece)
{
    return this->data & (1 << static_cast<u8>(piece));
};

void Bag::update(const piece::Type& next)
{
    this->data &= ~(1 << static_cast<u8>(next));
    
    if (this->data == 0) {
        this->data = 0b1111111;
    }
};

void Bag::deupdate(const piece::Type& next)
{
    if (this->data == 0b1111111) {
        this->data = 0;
    }

    this->data |= 1 << static_cast<u8>(next);
};

i32 Bag::get_size()
{
    return std::popcount(this->data);
};

void Bag::print()
{
    for (int i = 0; i < 7; ++i) {
        if (this->data & (1 << i)) {
            std::cout << piece::to_char(piece::Type(i)) << " ";
        }
    }

    std::cout << std::endl;
};