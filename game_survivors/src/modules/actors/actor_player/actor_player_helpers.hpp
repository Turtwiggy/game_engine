#pragma once

#include "components.hpp"
#include "steam/isteaminput.h"

namespace game2d {

InputComponent
generate_from_keyboard(entt::registry& r);

InputComponent
generate_from_handle(entt::registry& r, const InputHandle_t handle);

void
merge_inputs(InputComponent& i, const InputComponent& input);

} // namespace game2d