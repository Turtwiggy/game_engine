#pragma once

#include <string>

namespace engine {

// note: on mac, ends with a "/"
std::string
get_exe_path_without_exe_name();

} // namespace engine