// your header
#include "util_windows.hpp"

// stl
#include <iostream>

// other lib headers
#include "SDL_syswm.h"

namespace engine {

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
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);

  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  if (!SDL_GetWindowWMInfo(game_window.get_handle(), &wmi)) {
    std::cerr << "Failed getting native window handles: : " << std::string(SDL_GetError()) << std::endl;
    exit(0);
  }
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
  if (wmi.subsystem == SDL_SYSWM_WINDOWS) {
    auto* native_window = wmi.info.win.window;
    ofn.hwndOwner = native_window;
  }
#endif // defined(SDL_VIDEO_DRIVER_WINDOWS)

  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  printf("here0");
  if (GetOpenFileNameA(&ofn) == TRUE) {
    printf("here...!");
    return ofn.lpstrFile;
  }
  return std::string();
};

std::string
save_file(GameWindow& game_window, const char* filter)
{
  OPENFILENAMEA ofn;
  CHAR szFile[260] = { 0 };
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);

  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  if (!SDL_GetWindowWMInfo(game_window.get_handle(), &wmi)) {
    std::cerr << "Failed getting native window handles: : " << std::string(SDL_GetError()) << std::endl;
    exit(0);
  }
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
  if (wmi.subsystem == SDL_SYSWM_WINDOWS) {
    auto* native_window = wmi.info.win.window;
    ofn.hwndOwner = native_window;
  }
#endif // defined(SDL_VIDEO_DRIVER_WINDOWS)

  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  if (GetSaveFileNameA(&ofn) == TRUE) {
    return ofn.lpstrFile;
  }
  return std::string();
};

#endif

} // namespace engine