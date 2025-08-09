#pragma once

#include <entt/fwd.hpp>

namespace game2d {

std::string
get_confirm_button_str(entt::registry& r, InputHandle_t handle = 0);

std::string
get_back_button_str(entt::registry& r, InputHandle_t handle = 0);

} // namespace game2d