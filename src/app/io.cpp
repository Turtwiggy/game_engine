
// header
#include "io.hpp"

// other library headers
#include <chrono>
#include <iostream>
#include <string>

#ifdef WIN32
#include <Windows.h>
#endif

namespace engine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  const auto x = std::chrono::high_resolution_clock::now();
  const auto y = std::chrono::duration_cast<std::chrono::milliseconds>(x - start).count();
  std::cout << label << y << "ms"
            << "\n";
}

#ifdef WIN32

void
hide_windows_console()
{
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console
};

#endif

}