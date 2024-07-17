#pragma once

#include "entt/entt.hpp"
#include "imgui.h"

#include <string>
#include <utility>

namespace game2d {

std::pair<ImVec2, ImVec2>
convert_sprite_to_uv(entt::registry& r, const std::string& sprite);

} // namespace game2d