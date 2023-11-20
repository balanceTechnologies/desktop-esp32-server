#pragma once

#include <optional>
#include <string>
#include <variant>

namespace server {
namespace commands {
namespace incoming {
struct BalanceCoord {
  explicit BalanceCoord(unsigned x, unsigned y) : x{x}, y{y} {}
  unsigned const x;
  unsigned const y;
};

struct BalanceCoordMobile {
  explicit BalanceCoordMobile(unsigned x, unsigned y) : x{x}, y{y} {}
  unsigned const x;
  unsigned const y;
};

struct AllowMobile {};

struct BlockMobile {};

using Variant =
    std::variant<BalanceCoord, BalanceCoordMobile, AllowMobile, BlockMobile>;

std::optional<Variant> parse(std::string const &command);
} // namespace incoming

namespace outgoing {
struct Coord {
  explicit Coord(unsigned x, unsigned y) : x{x}, y{y} {}
  unsigned const x;
  unsigned const y;
};

struct BalanceCoord {
  explicit BalanceCoord(unsigned x, unsigned y) : x{x}, y{y} {}
  unsigned const x;
  unsigned const y;
};

struct ConnectionCount {
  explicit ConnectionCount(unsigned n) : n{n} {}
  unsigned const n;
};

struct ConnectionLost {};

using Variant =
    std::variant<Coord, BalanceCoord, ConnectionCount, ConnectionLost>;

std::string encode(Variant const &command);
} // namespace outgoing
} // namespace commands
} // namespace server
