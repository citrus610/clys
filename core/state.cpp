#include "state.h"

// Advances the game state
Lock State::advance(move::Placement& placement, const std::vector<piece::Type>& queue)
{
    auto current = queue[this->next];

    if (placement.type != current) {
        auto pre_hold = this->hold;
        this->hold = current;

        if (pre_hold == piece::Type::NONE) {
            this->bag.update(current);
            this->next++;
            current = queue[this->next];
        }
    }

    this->bag.update(current);
    this->next++;

    return this->lock(placement);
};

// Places the piece into the board and return the action's data
Lock State::lock(move::Placement& placement)
{
    Lock lock = Lock();

    lock.softdrop = !placement.is_above_stack(this->board);
    lock.tspin = placement.is_tspin(this->board);

    placement.place(this->board);

    lock.clear = this->board.clear();

    if (lock.clear > 0) {
        if (lock.tspin) {
            lock.attack = lock.clear * 2;
            this->b2b++;
        }
        else if (lock.clear == 4) {
            lock.attack = 4;
            this->b2b++;
        }
        else {
            lock.attack = lock.clear - 1;
            this->b2b = 0;
        }

        this->ren++;

        lock.attack += this->b2b > 1;
        lock.attack += REN_LUT[std::min(this->ren, u8(_countof(REN_LUT) - 1))];

        if (this->board.is_empty()) {
            lock.attack = 10;
        }
    }
    else {
        this->ren = 0;
    }

    this->b2b = std::min(this->b2b, u8(2));

    return lock;
};

// Hashes the state using xxh3 and return an u64
u64 State::get_hash()
{
    u8 buffer[84] = { 0 };

    memcpy(buffer, this->board.cols, 80);
    buffer[80] = this->bag.data;
    buffer[81] = static_cast<u8>(this->hold);
    buffer[82] = this->b2b;
    buffer[83] = this->ren;

    return xxh3_64((const void*)buffer, 84);
};

void State::print()
{
    printf("Board:\n");
    this->board.print();
    this->bag.print();
    printf("Hold:    %c\n", piece::to_char(this->hold));
    printf("Next:    %d\n", this->next);
    printf("B2b:     %d\n", this->b2b);
    printf("Ren:     %d\n", this->ren);
};