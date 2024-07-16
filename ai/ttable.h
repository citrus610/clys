#pragma once

#include "node.h"
#include "../lib/hash.h"

namespace Transposition
{

constexpr u32 DEFAULT_SIZE = 1 << 12;
constexpr u32 DEFAULT_BUCKET_SIZE = 8;

struct Entry
{
    u32 hash = 0;
    i32 acml = 0;
};

struct Bucket
{
    Entry slot[DEFAULT_BUCKET_SIZE] = { Entry() };
};

class Table
{
public:
    Bucket* bucket;
    u32 size;
public:
    Table();
    ~Table();
public:
    void init(u32 size = DEFAULT_SIZE);
    void destroy();
    void clear();
public:
    u32 hash(State& state);
public:
    bool get_entry(u32 hash, i32& acml);
    bool add_entry(u32 hash, i32 acml);
public:
    double hashful();
};

};