
// header
#include "util.hpp"

// engine header
#include "engine/opengl/texture.hpp"

// other library headers
#include <chrono>
#include <iostream>
#include <string>

// c/c++ lib headers
#ifdef WIN32
#include <Windows.h>
#include <commdlg.h>
#endif

namespace engine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  const auto x = std::chrono::high_resolution_clock::now();
  const auto y = std::chrono::duration_cast<std::chrono::milliseconds>(x - start).count();
  std::cout << label << y << "ms" << std::endl;
}

#ifdef WIN32

void
hide_windows_console()
{
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console
};

std::string
open_file(GameWindow& game_window, const char* filter)
{
  OPENFILENAMEA ofn;
  CHAR szFile[260] = { 0 };
  CHAR currentDir[256] = { 0 };
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);

  void* window = nullptr;
  game_window.get_native_handles(window);
  ofn.hwndOwner = (HWND)window;

  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  if (GetCurrentDirectoryA(256, currentDir))
    ofn.lpstrInitialDir = currentDir;
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  if (GetOpenFileNameA(&ofn) == TRUE) {
    return ofn.lpstrFile;
  }
  return std::string();
};

std::string
save_file(GameWindow& game_window, const char* filter)
{
  OPENFILENAMEA ofn;
  CHAR szFile[260] = { 0 };
  CHAR currentDir[256] = { 0 };
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);

  void* window = nullptr;
  game_window.get_native_handles(window);
  ofn.hwndOwner = (HWND)window;

  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  if (GetCurrentDirectoryA(256, currentDir))
    ofn.lpstrInitialDir = currentDir;
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
  if (GetSaveFileNameA(&ofn) == TRUE) {
    return ofn.lpstrFile;
  }
  return std::string();
};

#endif

}