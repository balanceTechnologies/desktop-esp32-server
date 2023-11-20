#pragma once

#include "esp32commands.hpp"

#include <asio.hpp>

namespace esp32 {
commands::incoming::Variant next_command(asio::serial_port &serial);
} // namespace esp32
