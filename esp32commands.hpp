#pragma once

#include <Qt>

#include <variant>

namespace esp32 {
namespace incoming {
struct Coord
{
    explicit Coord(quint32 x, quint32 y): x{x}, y{y} {}

    quint32 x;
    quint32 y;
};

struct BalanceCoord
{
    explicit BalanceCoord(quint32 x, quint32 y): x{x}, y{y} {}

    quint32 x;
    quint32 y;
};

using Command = std::variant<Coord, BalanceCoord>;

std::optional<Command> parse(const QByteArray &input);
} // namespace incoming

namespace outgoing {
struct SetBalanceCoord
{
    explicit SetBalanceCoord(qint32 x, qint32 y): x{x}, y{y} {}

    qint32 x;
    qint32 y;
};

struct MoveLeft {};
struct MoveRight {};
struct MoveForward {};
struct MoveBackward {};

using Command = std::variant<SetBalanceCoord, MoveLeft, MoveRight, MoveForward, MoveBackward>;

QString toString(const Command &command);
} // namespace outgoing
} // namespace esp32
