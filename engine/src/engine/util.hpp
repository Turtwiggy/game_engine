#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <vector>

namespace engine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start);

[[nodiscard]] std::vector<unsigned int>
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load);

}