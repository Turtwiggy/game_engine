#pragma once

#include "modules/ui_common/ui_common_components.hpp"

#include "steam/isteaminput.h"
#include <entt/fwd.hpp>

namespace game2d {

void
process_input_for_ui_all_handles(entt::registry& r, UIState& state);

// Note: if you use this variation, call
// state.new_actions.clear() before
void
process_input_for_ui(entt::registry& r, UIState& state, const InputHandle_t handle);

} // namespace game2d