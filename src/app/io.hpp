#pragma once

#include "game_window.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace engine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start);

// windows only

void
hide_windows_console();

} // namespace engine