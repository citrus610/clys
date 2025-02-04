#include <optional>
#include <array>
#include <string>
#include "stdint.h"
#include "../lib/nlohmann/json.hpp"

using nlohmann::json;

namespace nlohmann
{

template <typename T>
struct adl_serializer<std::optional<T>> {
    static void to_json(json& j, const std::optional<T>& opt) {
        if (opt.has_value()) {
            j = *opt;
        }
        else {
            j = nullptr;
        }
    }

    static void from_json(const json& j, std::optional<T>& opt) {
        if (j.is_null()) {
            opt = std::nullopt;
        }
        else {
            opt = j.get<T>();
        }
    }
};

}

namespace tbp 
{

enum class Piece 
{
    I,
    O,
    T,
    L,
    J,
    S,
    Z
};

NLOHMANN_JSON_SERIALIZE_ENUM(Piece, 
{
    {Piece::I, "I"},
    {Piece::O, "O"},
    {Piece::T, "T"},
    {Piece::L, "L"},
    {Piece::J, "J"},
    {Piece::S, "S"},
    {Piece::Z, "Z"}
})

enum class Cell 
{
    I,
    O,
    T,
    L,
    J,
    S,
    Z,
    G
};

NLOHMANN_JSON_SERIALIZE_ENUM(Cell, 
{
    {Cell::I, "I"},
    {Cell::O, "O"},
    {Cell::T, "T"},
    {Cell::L, "L"},
    {Cell::J, "J"},
    {Cell::S, "S"},
    {Cell::Z, "Z"},
    {Cell::G, "G"}
})

enum class Orientation 
{
    North,
    South,
    East,
    West
};

NLOHMANN_JSON_SERIALIZE_ENUM(Orientation, 
{
    {Orientation::North, "north"},
    {Orientation::South, "south"},
    {Orientation::East, "east"},
    {Orientation::West, "west"}
})

enum class Spin 
{
    None,
    Mini,
    Full
};

NLOHMANN_JSON_SERIALIZE_ENUM(Spin, 
{
    {Spin::None, "none"},
    {Spin::Mini, "mini"},
    {Spin::Full, "full"}
})

class PieceLocation 
{
public:
    Piece type;
    Orientation orientation;
    int32_t x;
    int32_t y;
public:
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PieceLocation,
        type,
        orientation,
        x,
        y
    )
};

class Move 
{
public:
    PieceLocation location;
    Spin spin;
public:
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Move,
        location,
        spin
    )
};

enum class ErrorCause 
{
    UnsupportedRules
};

NLOHMANN_JSON_SERIALIZE_ENUM(ErrorCause, 
{
    {ErrorCause::UnsupportedRules, "unsupported_rules"}
})

class Feature 
{
public:
    friend void to_json(json& j, const Feature& feature) {};
    friend void from_json(const json& j, Feature& feature) {};
};

namespace frontend 
{
    enum class FrontendMessageKind
    {
        Start,
        Stop,
        Suggest,
        Play,
        NewPiece,
        Rules,
        Quit
    };
    
    template <typename BasicJsonType>
    inline void to_json(BasicJsonType& j, const FrontendMessageKind& e) 
    {
        static_assert(std::is_enum<FrontendMessageKind>::value, "FrontendMessageKind must be an enum!");

        static const std::pair<FrontendMessageKind, BasicJsonType> m[] = 
        {
            {FrontendMessageKind::Start, "start"},
            {FrontendMessageKind::Stop, "stop"},
            {FrontendMessageKind::Suggest, "suggest"},
            {FrontendMessageKind::Play, "play"},
            {FrontendMessageKind::NewPiece, "new_piece"},
            {FrontendMessageKind::Rules, "rules"},
            {FrontendMessageKind::Quit, "quit"}
        };

        auto it = std::find_if
        (
            std::begin(m), 
            std::end(m),
            [e] (const std::pair<FrontendMessageKind, BasicJsonType>& ej_pair) -> bool { return ej_pair.first == e; }
        );

        j = ((it != std::end(m)) ? it : std::begin(m))->second;
    };

    template <typename BasicJsonType>
    inline void from_json(const BasicJsonType& j, FrontendMessageKind& e)
    {
        static_assert(std::is_enum<FrontendMessageKind>::value, "FrontendMessageKind must be an enum!");

        static const std::pair<FrontendMessageKind, BasicJsonType> m[] = 
        {
            {FrontendMessageKind::Start, "start"},
            {FrontendMessageKind::Stop, "stop"},
            {FrontendMessageKind::Suggest, "suggest"},
            {FrontendMessageKind::Play, "play"},
            {FrontendMessageKind::NewPiece, "new_piece"},
            {FrontendMessageKind::Rules, "rules"},
            {FrontendMessageKind::Quit, "quit"}
        };

        auto it = std::find_if
        (
            std::begin(m), 
            std::end(m),
            [&j] (const std::pair<FrontendMessageKind, BasicJsonType>& ej_pair) -> bool { return ej_pair.second == j; }
        );

        e = ((it != std::end(m)) ? it : std::begin(m))->first;
    };

    class Randomizer 
    {
    public:
        std::vector<Piece> bag_state;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Randomizer,
            bag_state
        )
    };

    class Start 
    {
    public:
        std::optional<Piece> hold;
        std::vector<Piece> queue;
        uint32_t combo;
        bool back_to_back;
        std::array<std::array<std::optional<Cell>, 10>, 40> board;
        Randomizer randomizer;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Start,
            hold,
            queue,
            combo,
            back_to_back, 
            board,
            randomizer
        )
    };

    class Play 
    {
    public:
        Move move;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Play,
            move
        )
    };

    class NewPiece 
    {
    public:
        Piece piece;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NewPiece,
            piece
        )
    };

};

namespace bot 
{
    enum class BotMessageKind 
    {
        Error,
        Ready,
        Info,
        Suggestion
    };
    
    NLOHMANN_JSON_SERIALIZE_ENUM(BotMessageKind, 
    {
        {BotMessageKind::Error, "error"},
        {BotMessageKind::Ready, "ready"},
        {BotMessageKind::Info, "info"},
        {BotMessageKind::Suggestion, "suggestion"}
    })

    class Ready 
    {
    public:
        friend void to_json(json& j, const Ready& t) {
            j = json();
        };
        friend void from_json(const json& j, Ready& t) {};
    };

    class Error 
    {
    public:
        ErrorCause reason;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Error,
            reason
        )
    };

    class Info 
    {
    public:
        std::string name;
        std::string version;
        std::string author;
        std::vector<Feature> features;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Info,
            name,
            version,
            author,
            features
        )
    };

    class Suggestion 
    {
    public:
        struct MoveInfo {
            uint32_t nodes = 0;
            uint32_t nps = 0;
            uint32_t depth = 0;
            std::string extra = "";

            NLOHMANN_DEFINE_TYPE_INTRUSIVE(MoveInfo,
                nodes, nps, depth, extra
            )
        };
    public:
        std::vector<Move> moves;
        MoveInfo move_info;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Suggestion,
            moves, move_info
        )
    };
    
};

};