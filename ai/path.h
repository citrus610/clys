#pragma once

#include "../core/piece.h"
#include "../core/board.h"

namespace Path
{

enum class Move
{
    NONE,
    RIGHT,
    LEFT,
    CW,
    CCW,
    DOWN,
    DROP
};

enum class Input
{
    NONE,
    RIGHT,
    LEFT,
    CW,
    CCW,
    DOWN,
    DROP,
    DROP_CW,
    DROP_CCW,
    WAIT
};

typedef std::vector<Input> Queue;

class Node
{
public:
    Piece::Data placement;
    std::vector<Move> move;
    i32 time;
public:
    Node();
public:
    bool attempt(Board& board, Move move);
public:
    bool operator < (Node& other);
    bool operator > (Node& other);
    bool operator == (Node& other);
};

class Map
{
public:
    Node data[10][40][4];
public:
    Map();
public:
    bool get(Piece::Data placement, Node& node);
    bool add(Piece::Data placement, Node& node);
public:
    void clear();
};

std::vector<Move> search(Board board, Piece::Data destination);

void expand(Board board, Node& node, std::vector<Node>& queue, Map& map_queue);

void lock(Board board, Node& node, std::vector<Node>& locks, Map& map_locks);

void add(Node& node, std::vector<Node>& queue, Map& map_queue);

int index(Node& node, std::vector<Node>& queue);

static std::string convert_move_to_str(Move move)
{
    switch (move)
    {
    case Move::NONE:
        return " ";
        break;
    case Move::RIGHT:
        return "RIGHT";
        break;
    case Move::LEFT:
        return "LEFT";
        break;
    case Move::CW:
        return "CW";
        break;
    case Move::CCW:
        return "CCW";
        break;
    case Move::DOWN:
        return "DOWN";
        break;
    case Move::DROP:
        return "DROP";
        break;
    default:
        break;
    }

    return " ";
};

static Queue convert_move_to_input(std::vector<Move>& move)
{
    Queue input;
    input.clear();

    for (int i = 0; i < int(move.size()); ++i) {
        if (i > 0 && move[i] == move[i - 1]) {
            input.push_back(Input::NONE);
        }

        switch (move[i])
        {
        case Move::RIGHT:
            input.push_back(Input::RIGHT);
            break;
        case Move::LEFT:
            input.push_back(Input::LEFT);
            break;
        case Move::CW:
            input.push_back(Input::CW);
            break;
        case Move::CCW:
            input.push_back(Input::CCW);
            break;
        case Move::DOWN:
            input.push_back(Input::DOWN);
            break;
        default:
            break;
        }
    }

    if (!input.empty() && input.back() == Input::DOWN) {
        input.pop_back();
    }
    input.push_back(Input::DROP);

    if (input.size() < 2) {
        return input;
    }

    if (input[input.size() - 2] == Input::CW) {
        input.pop_back();
        input.pop_back();
        input.push_back(Input::DROP_CW);
    }
    else if (input[input.size() - 2] == Input::CCW)
    {
        input.pop_back();
        input.pop_back();
        input.push_back(Input::DROP_CCW);
    }

    return input;
};

static void print_path(Board board, Piece::Data destination) {
    using namespace std;

    vector<Move> move = search(board, destination);

    std::string move_str;
    for (int i = 0; i < int(move.size()); ++i) {
        move_str += convert_move_to_str(move[i]) + " ";
    }

    Piece::Data piece = Piece::Data(4, 19, destination.type, Piece::Rotation::UP);
    if (board.is_colliding(piece)) {
        piece.y += 1;
    }

    {
        Board copy = board;
        copy.place_piece(piece);
        copy.print();
        cin.get();
        system("cls");
    }
    for (int i = 0; i < int(move.size()); ++i) {
        Board copy = board;
        switch (move[i])
        {
        case Move::RIGHT:
            piece.move_right(board);
            break;
        case Move::LEFT:
            piece.move_left(board);
            break;
        case Move::CW:
            piece.move_cw(board);
            break;
        case Move::CCW:
            piece.move_ccw(board);
            break;
        case Move::DOWN:
            piece.move_drop(board);
            break;
        default:
            break;
        }

        copy.place_piece(piece);
        copy.print();
        cout << move_str << endl;
        cin.get();
        system("cls");
    }
};

};