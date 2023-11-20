#include "esp32commands.hpp"

#include <optional>
#include <sstream>
#include <string>
#include <variant>

namespace esp32 {
namespace commands {
namespace incoming {
std::optional<Variant> parse(std::string const &command) {
  std::istringstream is(command);

  std::string token;
  if (!(is >> token)) {
    return std::nullopt;
  }

  if (token == "coord") {
    unsigned x;
    unsigned y;

    if ((is >> x >> y) && !(is >> token)) {
      return Coord(x, y);
    }
  } else if (token == "balance_coord") {
    unsigned x;
    unsigned y;

    if ((is >> x >> y) && !(is >> token)) {
      return BalanceCoord(x, y);
    }
  }

  return std::nullopt;
}
} // namespace incoming

namespace outgoing {
std::string encode(Variant const &command) {
  struct Encoder {
    std::string operator()(BalanceCoord const &command) const {
      std::ostringstream oss;
      oss << "balance_coord " << command.x << ' ' << command.y;
      return oss.str();
    }
  };

  return std::visit(Encoder{}, command);
}
} // namespace outgoing
} // namespace commands
} // namespace esp32
