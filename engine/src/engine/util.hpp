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

uint64_t
encode_cantor_pairing_function(int x, int y);

void
decode_cantor_pairing_function(uint64_t p, uint32_t& x, uint32_t& y);

}