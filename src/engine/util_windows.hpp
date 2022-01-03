#pragma once

#include "game_window.hpp"

// c/c++ lib headers
#ifdef WIN32
#include <Windows.h>
#include <commdlg.h>
#endif

#include <string>

namespace engine {

void
hide_windows_console();

std::string
open_file(GameWindow& game_window, const char* filter);

std::string
save_file(GameWindow& game_window, const char* filter);

} // namespace engine