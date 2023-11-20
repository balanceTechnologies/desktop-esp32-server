#include "esp32commands.hpp"
#include "esp32.hpp"

#include <asio.hpp>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

int main(int argc, char **argv) {
  char const *device = "/dev/ttyACM0";
  unsigned int baud_rate = 115200;

  if (argc > 3) {
    std::cerr << "wrong number of arguments\n";
    return 1;
  }

  if (argc >= 2) {
    device = argv[1];
  }

  if (argc >= 3) {
    char const *baud_str = argv[2];

    char *endptr;
    unsigned long arg_baud_rate = std::strtoul(baud_str, &endptr, 10);
    if (endptr != baud_str && *endptr == '\0') {
      baud_rate = arg_baud_rate;
    } else {
      std::cerr << "faulty baud rate format\n";
      return 1;
    }
  }

  asio::io_service io;
  asio::serial_port serial(io, device);
  serial.set_option(asio::serial_port_base::baud_rate(baud_rate));

  for (;;) {
    using namespace esp32::commands::incoming;
    struct Printer {
      void operator()(Coord const &command) const {
        std::cout << "coord " << command.x << ' ' << command.y << '\n';
      }

      void operator()(BalanceCoord const &command) const {
        std::cout << "balance_coord " << command.x << ' ' << command.y << '\n';
      }
    };
    auto command = esp32::next_command(serial);

    std::visit(Printer{}, command);
  }
}
