#include "servercommands.hpp"

#include <optional>
#include <sstream>
#include <string>
#include <variant>

namespace server {
namespace commands {
namespace incoming {
std::optional<Variant> parse(std::string const &command) {
  std::istringstream is(command);

  std::string token;
  if (!(is >> token)) {
    return std::nullopt;
  }

  if (token == "balance_coord") {
    unsigned x;
    unsigned y;

    if ((is >> x >> y) && !(is >> token)) {
      return BalanceCoord(x, y);
    }
  } else if (token == "balance_coord_mobile") {
    unsigned x;
    unsigned y;

    if ((is >> x >> y) && !(is >> token)) {
      return BalanceCoordMobile(x, y);
    }
  } else if (token == "allow_mobile") {
    if (!(is >> token)) {
      return AllowMobile();
    }
  } else if (token == "block_mobile") {
    if (!(is >> token)) {
      return BlockMobile();
    }
  }

  return std::nullopt;
}
} // namespace incoming

namespace outgoing {
std::string encode(Variant const &command) {
  struct Encoder {
    std::string operator()(Coord const &command) const {
      std::ostringstream oss;
      oss << "coord " << command.x << ' ' << command.y;
      return oss.str();
    }

    std::string operator()(BalanceCoord const &command) const {
      std::ostringstream oss;
      oss << "balance_coord " << command.x << ' ' << command.y;
      return oss.str();
    }

    std::string operator()(ConnectionCount const &command) const {
      std::ostringstream oss;
      oss << "connection_count " << command.n;
      return oss.str();
    }

    std::string operator()(ConnectionLost const &command) const {
      return "connection_lost";
    }
  };

  return std::visit(Encoder{}, command);
}
} // namespace outgoing
} // namespace commands
} // namespace server
