
// header
#include "io.hpp"

// other library headers
#include <chrono>
#include <print>
#include <string>

#if defined(WIN32)
#include <Windows.h>
#endif

namespace engine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  const auto x = std::chrono::high_resolution_clock::now();
  const auto y = std::chrono::duration_cast<std::chrono::milliseconds>(x - start).count();
  std::println("{}{}ms", label, y);
}

#if defined(WIN32)

void
hide_windows_console()
{
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console
};

#endif

}