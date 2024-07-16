#include <iostream>
#include <cstdint>
#include <array>

#include "../lib/nlohmann/json.hpp"
#include "tbp.h"
#include "../ai/ai.h"

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

Piece::Type convert_piece_from_tbp(tbp::Piece piece)
{
    switch (piece)
    {
    case tbp::Piece::I:
        return Piece::Type::I;
    case tbp::Piece::J:
        return Piece::Type::J;
    case tbp::Piece::L:
        return Piece::Type::L;
    case tbp::Piece::O:
        return Piece::Type::O;
    case tbp::Piece::T:
        return Piece::Type::T;
    case tbp::Piece::S:
        return Piece::Type::S;
    case tbp::Piece::Z:
        return Piece::Type::Z;
    default:
        return Piece::Type::NONE;
    }
    return Piece::Type::NONE;
};

tbp::Piece convert_piece_to_tbp(Piece::Type piece)
{
    switch (piece)
    {
    case Piece::Type::I:
        return tbp::Piece::I;
    case Piece::Type::J:
        return tbp::Piece::J;
    case Piece::Type::L:
        return tbp::Piece::L;
    case Piece::Type::O:
        return tbp::Piece::O;
    case Piece::Type::T:
        return tbp::Piece::T;
    case Piece::Type::S:
        return tbp::Piece::S;
    case Piece::Type::Z:
        return tbp::Piece::Z;
    }
    return tbp::Piece::I;
};

Piece::Rotation convert_rotation_from_tbp(tbp::Orientation rotation)
{
    switch (rotation)
    {
    case tbp::Orientation::North:
        return Piece::Rotation::UP;
    case tbp::Orientation::East:
        return Piece::Rotation::RIGHT;
    case tbp::Orientation::South:
        return Piece::Rotation::DOWN;
    case tbp::Orientation::West:
        return Piece::Rotation::LEFT;
    }
    return Piece::Rotation::UP;
};

tbp::Orientation convert_rotation_to_tbp(Piece::Rotation rotation)
{
    switch (rotation)
    {
    case Piece::Rotation::UP:
        return tbp::Orientation::North;
    case Piece::Rotation::RIGHT:
        return tbp::Orientation::East;
    case Piece::Rotation::DOWN:
        return tbp::Orientation::South;
    case Piece::Rotation::LEFT:
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
    to_json(js, Evaluation::DEFAULT);
    o << std::setw(4) << js << std::endl;
    o.close();
};

void load(Evaluation::Weight& w)
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
        .name = "mojito",
        .version = "v0.1",
        .author = "citrus610",
        .features {}
    };

    send_message(BotMessageKind::Info, info);

    AI::Engine ai = AI::Engine();
    std::vector<Piece::Type> new_piece;

    Evaluation::Weight w = Evaluation::DEFAULT;

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
            std::vector<Piece::Type> queue;
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
            Piece::Type hold = Piece::Type::NONE;
            if (auto message_hold = start_message.hold) {
                hold = convert_piece_from_tbp(*message_hold);
            }

            // Set bag
            Bag bag;
            memset(bag.data, false, 7 * sizeof(bool));
            for (auto piece : start_message.randomizer.bag_state) {
                bag.data[static_cast<u8>(convert_piece_from_tbp(piece))] = true;
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

            ai.init(queue, Lock(), state, w);

            ai.search();

            break;
        }

        case FrontendMessageKind::Suggest:
        {
            auto plan = ai.request(0);

            while (!plan.has_value())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                plan = ai.request(0);
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

            Piece::Data piece = Piece::Data
            (
                i8(play_message.move.location.x),
                i8(play_message.move.location.y),
                convert_piece_from_tbp(play_message.move.location.type),
                convert_rotation_from_tbp(play_message.move.location.orientation)
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
            // std::cerr << "Stop ai...\n";
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