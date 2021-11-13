#pragma once

#ifdef WIN32
#include <Windows.h>
#include <commdlg.h>
#endif

namespace engine {

inline void
hide_windows_console()
{
#ifdef WIN32
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console
#endif
};

//   static std::string OpenFile(const char* filter);
//   static std::string SaveFile(const char* filter);

}