#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <vector>

namespace fightingengine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start);

void
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load,
                       const std::chrono::steady_clock::time_point& app_start);

}