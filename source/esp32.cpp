#include "esp32.hpp"

namespace esp32 {
static std::string read_command(asio::serial_port &serial) {
  std::string command;
  bool in_curly = false;
  for (;;) {
    char c;
    asio::read(serial, asio::buffer(&c, 1));

    switch (c) {
    case '{':
      if (in_curly) {
        command.clear();
      } else {
        in_curly = true;
      }
      break;
    case '}':
      if (in_curly) {
        return command;
      }
      break;
    default:
      if (in_curly) {
        command.push_back(c);
      }
      break;
    }
  }
}

commands::incoming::Variant next_command(asio::serial_port &serial) {
  for (;;) {
    std::string command = read_command(serial);
    auto next = commands::incoming::parse(command);
    if (next) {
      return *next;
    }
  }
}
} // namespace esp32
