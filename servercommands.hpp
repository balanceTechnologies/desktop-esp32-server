#pragma once

#include <Qt>

#include <variant>

namespace server {
namespace incoming {
struct ConnectDesktop
{};

struct ConnectMobile
{};

struct SetBalanceCoord
{
    explicit SetBalanceCoord(quint32 x, quint32 y)
        : x{x}
        , y{y}
    {}

    quint32 x;
    quint32 y;
};

// TEMPORARY
struct MoveLeft {};
struct MoveRight {};
struct MoveForward {};
struct MoveBackward {};

using Command = std::variant<ConnectDesktop, ConnectMobile, SetBalanceCoord, MoveLeft, MoveRight, MoveForward, MoveBackward>;
} // namespace incoming

namespace outgoing {
struct Coord
{
    explicit Coord(quint32 x, quint32 y)
        : x{x}
        , y{y}
    {}

    quint32 x;
    quint32 y;
};

struct BalanceCoord
{
    explicit BalanceCoord(quint32 x, quint32 y)
        : x{x}
        , y{y}
    {}

    quint32 x;
    quint32 y;
};

using Command = std::variant<Coord, BalanceCoord>;
} // namespace outgoing
} // namespace server
