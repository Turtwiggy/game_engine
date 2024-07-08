#include "path.hpp"

#include "fmt/core.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace engine {
using namespace std::literals;

std::string
get_exe_path_without_exe_name()
{
  std::string path = "";

#if defined(__APPLE__)
  char path_buf[1024];
  uint32_t size = sizeof(path_buf);
  if (_NSGetExecutablePath(path_buf, &size) == 0)
    path = std::string(path_buf);
  else
    fmt::println("buffer too small; need size: {}", size);

  // Remove the exectable name from the exe path
  const size_t pos = path.find_last_of('/');
  if (pos != std::string::npos)
    path = path.substr(0, pos);

  if (!path.ends_with('/'))
    path += "/"s;

#endif

  return path;
};

} // namespace engine