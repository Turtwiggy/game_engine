#pragma once

#include "components.hpp"
#include "steam/isteaminput.h"

namespace game2d {

InputComponent
generate_from_handle(entt::registry& r, const InputHandle_t handle);

} // namespace game2d