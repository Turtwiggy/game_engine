#pragma once

#include "game_window.hpp"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

namespace engine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start);

// windows only

void
hide_windows_console();

std::string
open_file(GameWindow& game_window, const char* filter);

std::string
save_file(GameWindow& game_window, const char* filter);

}