#include "state.h"

Lock State::advance(move::Placement& placement, const std::vector<piece::Type>& queue)
{
    auto current = queue[this->next];

    if (placement.type != current) {
        auto hold_old = this->hold;
        this->hold = current;

        if (hold_old == piece::Type::NONE) {
            this->bag.update(current);
            this->next += 1;

            current = queue[this->next];
        }
    }

    this->bag.update(current);
    this->next += 1;

    return this->lock(placement);
};

Lock State::lock(move::Placement& placement)
{
    Lock lock = Lock();

    lock.softdrop = !placement.is_above_stack(this->board);
    lock.tspin = placement.is_tspin(this->board);

    placement.place(this->board);

    lock.clear = this->board.clear_lines();

    if (lock.clear > 0) {
        if (lock.tspin) {
            lock.attack = lock.clear * 2;
            this->b2b += 1;
        }
        else if (lock.clear == 4) {
            lock.attack = 4;
            this->b2b += 1;
        }
        else {
            lock.attack = lock.clear - 1;
            this->b2b = 0;
        }

        this->ren += 1;

        lock.attack += this->b2b > 1;
        lock.attack += REN_LUT[std::min(this->ren, u8(12))];

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

u64 State::get_hash()
{
    u8 buffer[84] = { 0 };

    memcpy(buffer, this->board.data, 80);
    buffer[80] = this->bag.data;
    buffer[81] = static_cast<u8>(this->hold);
    buffer[82] = this->b2b;
    buffer[83] = this->ren;

    return rapidhash((const void*)buffer, 84);
};

void State::print()
{
    printf("Board:\n");
    this->board.print();
    this->bag.print();
    printf("Hold:    %c\n", piece::get_char(this->hold));
    printf("Next:    %d\n", this->next);
    printf("B2b:     %d\n", this->b2b);
    printf("Ren:     %d\n", this->ren);
};