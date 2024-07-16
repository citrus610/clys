#include "state.h"

Lock State::advance(Piece::Data& placement, std::vector<Piece::Type>& queue)
{
    auto current = queue[this->next];

    if (placement.type != current) {
        auto pre_hold = this->hold;
        this->hold = current;

        if (pre_hold == Piece::Type::NONE) {
            this->bag.update(current);
            this->next++;
            current = queue[this->next];
        }
    }

    this->bag.update(current);
    this->next++;

    return this->lock(placement);
};

Lock State::lock(Piece::Data& placement)
{
    Lock lock = Lock();

    lock.softdrop = !this->board.is_above_stack(placement);
    lock.tspin = this->board.is_tspin(placement);

    this->board.place_piece(placement);

    lock.clear = this->board.clear_line();

    if (lock.clear > 0) {
        this->ren++;

        if (lock.tspin || lock.clear == 4) {
            this->b2b++;
        }
        else {
            this->b2b = 0;
        }

        lock.pc = this->board.is_perfect();
    }
    else {
        this->ren = 0;
    }

    this->b2b = std::min(this->b2b, 4);

    return lock;
};

void State::print()
{
    using namespace std;

    cout << "Board:" << endl;
    this->board.print();
    cout << "Hold:    " << Piece::to_char(this->hold) << endl;
    cout << "Next:    " << this->next << endl;
    cout << "Bag:     ";
    this->bag.print();
    cout << "B2b:     " << this->b2b << endl;
    cout << "Ren:     " << this->ren << endl;
};