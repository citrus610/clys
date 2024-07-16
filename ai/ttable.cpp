#include "ttable.h"

namespace Transposition
{

Table::Table()
{
    this->bucket = nullptr;
    this->size = 0;
};

Table::~Table()
{
    this->destroy();
};

void Table::init(u32 size)
{
    assert((size & (size - 1)) == 0);
    if (this->bucket != nullptr) {
        assert(false);
        return;
    }
    this->size = size;
    this->bucket = new Bucket[this->size];
    this->clear();
};

void Table::destroy()
{
    if (this->bucket == nullptr) {
        return;
    }
    delete[] this->bucket;
};

void Table::clear()
{
    memset(this->bucket, 0, sizeof(Bucket) * this->size);
};

u32 Table::hash(State& state)
{
    u8 buffer[90] = { 0 };
    memcpy(buffer, state.board.cols, 80);
    memcpy(buffer + 80, state.bag.data, 7);
    buffer[87] = static_cast<u8>(state.hold);
    buffer[88] = u8(state.b2b & 0xFF);
    buffer[89] = u8(state.ren & 0xFF);
    return xxhash32((const void*)buffer, 90, 31);
};

bool Table::get_entry(u32 hash, i32& acml)
{
    u32 index = hash & (this->size - 1);
    for (u32 i = 0; i < DEFAULT_BUCKET_SIZE; ++i) {
        if (this->bucket[index].slot[i].hash == hash) {
            acml = this->bucket[index].slot[i].acml;
            return true;
        }
    }
    return false;
};

bool Table::add_entry(u32 hash, i32 acml)
{
    u32 index = hash & (this->size - 1);
    i32 slot_empty = -1;
    i32 slot_smallest = -1;
    for (u32 i = 0; i < DEFAULT_BUCKET_SIZE; ++i) {
        if (this->bucket[index].slot[i].hash == hash) {
            if (this->bucket[index].slot[i].acml < acml) {
                this->bucket[index].slot[i].acml = acml;
                return true;
            }
            return false;
        }
        if (this->bucket[index].slot[i].hash == 0 && this->bucket[index].slot[i].acml == 0) {
            slot_empty = i32(i);
            continue;
        }
        if (slot_smallest == -1 || this->bucket[index].slot[i].acml < this->bucket[index].slot[slot_smallest].acml) {
            slot_smallest = i32(i);
        }
    }
    if (slot_empty != -1) {
        this->bucket[index].slot[slot_empty].hash = hash;
        this->bucket[index].slot[slot_empty].acml = acml;
        return true;
    }
    if (slot_smallest != -1) {
        this->bucket[index].slot[slot_smallest].hash = hash;
        this->bucket[index].slot[slot_smallest].acml = acml;
        return true;
    }
    return false;
};

double Table::hashful()
{
    u32 total = 0;
    for (u32 i = 0; i < this->size; ++i) {
        for (u32 k = 0; k < DEFAULT_BUCKET_SIZE; ++k) {
            if (this->bucket[i].slot[k].hash != 0 || this->bucket[i].slot[k].acml != 0) {
                total += 1;
            }
        }
    }
    return double(total) / double(this->size * DEFAULT_BUCKET_SIZE);
};

};