#pragma once

#include "components.hpp"
#include "steam/isteaminput.h"

namespace game2d {

bool
has_action(const std::vector<ActionStateEnum>& action, const ActionStateEnum act);

InputComponent
generate_from_handle(entt::registry& r, const InputHandle_t handle);

} // namespace game2d