#include <iostream>
#include <cstdint>
#include <array>

#include "../lib/nlohmann/json.hpp"
#include "../ai/ai.h"
#include "tbp.h"

using nlohmann::json;
using tbp::frontend::FrontendMessageKind;
using tbp::bot::BotMessageKind;

template <typename M>
void send_message(BotMessageKind kind, M message)
{
	json messageJson = message;
	messageJson["type"] = kind;
	std::cout << messageJson << std::endl;
};

piece::Type convert_piece_from_tbp(tbp::Piece piece)
{
    switch (piece)
    {
    case tbp::Piece::I:
        return piece::Type::I;
    case tbp::Piece::J:
        return piece::Type::J;
    case tbp::Piece::L:
        return piece::Type::L;
    case tbp::Piece::O:
        return piece::Type::O;
    case tbp::Piece::T:
        return piece::Type::T;
    case tbp::Piece::S:
        return piece::Type::S;
    case tbp::Piece::Z:
        return piece::Type::Z;
    default:
        return piece::Type::NONE;
    }
    return piece::Type::NONE;
};

tbp::Piece convert_piece_to_tbp(piece::Type piece)
{
    switch (piece)
    {
    case piece::Type::I:
        return tbp::Piece::I;
    case piece::Type::J:
        return tbp::Piece::J;
    case piece::Type::L:
        return tbp::Piece::L;
    case piece::Type::O:
        return tbp::Piece::O;
    case piece::Type::T:
        return tbp::Piece::T;
    case piece::Type::S:
        return tbp::Piece::S;
    case piece::Type::Z:
        return tbp::Piece::Z;
    default:
        return tbp::Piece::I;
    }

    return tbp::Piece::I;
};

piece::Rotation convert_rotation_from_tbp(tbp::Orientation rotation)
{
    switch (rotation)
    {
    case tbp::Orientation::North:
        return piece::Rotation::UP;
    case tbp::Orientation::East:
        return piece::Rotation::RIGHT;
    case tbp::Orientation::South:
        return piece::Rotation::DOWN;
    case tbp::Orientation::West:
        return piece::Rotation::LEFT;
    }
    return piece::Rotation::UP;
};

tbp::Orientation convert_rotation_to_tbp(piece::Rotation rotation)
{
    switch (rotation)
    {
    case piece::Rotation::UP:
        return tbp::Orientation::North;
    case piece::Rotation::RIGHT:
        return tbp::Orientation::East;
    case piece::Rotation::DOWN:
        return tbp::Orientation::South;
    case piece::Rotation::LEFT:
        return tbp::Orientation::West;
    }
    return tbp::Orientation::North;
};

void save()
{
    std::ifstream f("config.json");

    if (f.good()) {
        return;
    };

    f.close();

    std::ofstream o("config.json");
    json js;
    to_json(js, eval::Weight());
    o << std::setw(4) << js << std::endl;
    o.close();
};

void load(eval::Weight& w)
{
    std::ifstream file;
    file.open("config.json");
    json js;
    file >> js;
    file.close();
    from_json(js, w);
};

int main()
{
    tbp::bot::Info info
    {
        .name = "clys",
        .version = "v1.5",
        .author = "citrus610",
        .features {}
    };

    send_message(BotMessageKind::Info, info);

    ai::Engine ai = ai::Engine();
    std::vector<piece::Type> new_piece;

    eval::Weight w = eval::Weight();

    save();
    load(w);

    while (true)
    {
        json message;
        std::cin >> message;

        switch (message["type"].get<FrontendMessageKind>())
        {

        case FrontendMessageKind::Rules:
        {
            send_message(BotMessageKind::Ready, tbp::bot::Ready());
            break;
        }

        case FrontendMessageKind::Start:
        {
            ai.clear();

            new_piece.clear();

            auto start_message = message.get<tbp::frontend::Start>();

            // Set queue
            std::vector<piece::Type> queue;
            for (auto piece : start_message.queue) {
                queue.push_back(convert_piece_from_tbp(piece));
            }

            // Set board
            Board board;
            for (size_t y = 0; y < start_message.board.size(); y++) {
                auto& row = start_message.board[y];
                for (size_t x = 0; x < row.size(); x++) {
                    if (row[x].has_value()) {
                        board[x] |= 1ULL << y;
                    }
                }
            }

            // Set hold
            piece::Type hold = piece::Type::NONE;
            if (auto message_hold = start_message.hold) {
                hold = convert_piece_from_tbp(*message_hold);
            }

            // Set bag
            Bag bag;

            bag.data = 0;
            
            for (auto piece : start_message.randomizer.bag_state) {
                bag.data |= 1U << static_cast<u8>(convert_piece_from_tbp(piece));
            }

            for (int i = int(queue.size()) - 1; i >=0; --i) {
                bag.deupdate(queue[i]);
            }

            // Set b2b
            int b2b = start_message.back_to_back;

            // Set ren
            int ren = start_message.combo + 1;

            // Set state
            State state = State();
            state.board = board;
            state.hold = hold;
            state.bag = bag;
            state.next = 0;
            state.b2b = b2b;
            state.ren = ren;

            ai.init(queue, state, Lock(), w);

            ai.search();

            break;
        }

        case FrontendMessageKind::Suggest:
        {
            auto plan = ai.request(0);

            if (!plan.has_value()) {
                return -1;
            }

            tbp::bot::Suggestion suggestion;

            std::array spins
            {
                tbp::Spin::Full,
                tbp::Spin::None,
                tbp::Spin::Mini
            };

            for (auto spin : spins) {
                suggestion.moves.push_back
                ({
                    .location = {
                        .type = convert_piece_to_tbp(plan->placement.type),
                        .orientation = convert_rotation_to_tbp(plan->placement.r),
                        .x = plan->placement.x,
                        .y = plan->placement.y
                    },
                    .spin = spin
                });
            }

            suggestion.move_info.nodes = plan->nodes;
            suggestion.move_info.depth = plan->depth;
            suggestion.move_info.nps = 0;
            suggestion.move_info.extra = std::to_string(plan->eval);

            send_message(BotMessageKind::Suggestion, suggestion);

            break;
        }

        case FrontendMessageKind::Play:
        {
            auto play_message = message.get<tbp::frontend::Play>();

            auto piece = move::Placement(
                i8(play_message.move.location.x),
                i8(play_message.move.location.y),
                convert_rotation_from_tbp(play_message.move.location.orientation),
                convert_piece_from_tbp(play_message.move.location.type)
            );

            if (!ai.advance(piece, new_piece)) {
                std::cerr << "Error advancing state...\n";
                ai.request(0);
                ai.clear();
                return 0;
            }

            new_piece.clear();

            ai.search();

            break;
        }

        case FrontendMessageKind::NewPiece: 
        {
            auto new_piece_message = message.get<tbp::frontend::NewPiece>();
            new_piece.push_back(convert_piece_from_tbp(new_piece_message.piece));
            break;
        }

        case FrontendMessageKind::Stop:
        {
            ai.request(0);
            ai.clear();
            break;
        }
        
        case FrontendMessageKind::Quit: {
            ai.request(0);
            ai.clear();
            return 0;
        }

        default:
            break;
        }
    }

    return 0;
}