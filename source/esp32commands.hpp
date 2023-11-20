#pragma once

#include <optional>
#include <string>
#include <variant>

namespace esp32 {
namespace commands {
namespace incoming {
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

using Variant = std::variant<Coord, BalanceCoord>;

std::optional<Variant> parse(std::string const &command);
} // namespace incoming

namespace outgoing {
struct BalanceCoord {
  explicit BalanceCoord(unsigned x, unsigned y) : x{x}, y{y} {}
  unsigned const x;
  unsigned const y;
};

using Variant = std::variant<BalanceCoord>;

std::string encode(Variant const &command);
} // namespace outgoing
} // namespace commands
} // namespace esp32
