#pragma once

#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace game2d {

std::vector<std::string>
load_file_into_lines(const std::string& path);

} // namespace game2d