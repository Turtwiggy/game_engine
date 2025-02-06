#pragma once

#include "modules/ui_common/ui_common_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
process_input_for_ui(entt::registry& r, UIState& state);

} // namespace game2d