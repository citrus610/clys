#include "bag.h"

Bag::Bag()
{
    memset(this->data, true, 7);
};

Bag::Bag(bool init[7])
{
    memcpy(this->data, init, 7);
};

bool& Bag::operator [] (i32 index)
{
    return this->data[index];
};

void Bag::update(Piece::Type next)
{
    assert(this->data[static_cast<i8>(next)]);
    this->data[static_cast<i8>(next)] = false;
    if (this->data[0] == false &&
        this->data[1] == false &&
        this->data[2] == false &&
        this->data[3] == false &&
        this->data[4] == false &&
        this->data[5] == false &&
        this->data[6] == false) {
        memset(this->data, true, 7);
    }
};

void Bag::deupdate(Piece::Type next)
{
    if (this->data[0] == true &&
        this->data[1] == true &&
        this->data[2] == true &&
        this->data[3] == true &&
        this->data[4] == true &&
        this->data[5] == true &&
        this->data[6] == true) {
        memset(this->data, false, 7);
    }
    this->data[static_cast<i8>(next)] = true;
};

i32 Bag::get_size()
{
    i32 result = this->data[0];
    for (i32 i = 1; i < 7; ++i) {
        result += this->data[i];
    }
    return result;
};

void Bag::print()
{
    using namespace std;

    for (int i = 0; i < 7; ++i) {
        if (this->data[i]) {
            cout << Piece::to_char(Piece::Type(i)) << " ";
        }
    }

    cout << endl;
};