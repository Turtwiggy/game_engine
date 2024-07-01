#pragma once

#include <chrono>
#include <string>

namespace engine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start);

#if defined(WIN32)

void
hide_windows_console();

#endif

} // namespace engine