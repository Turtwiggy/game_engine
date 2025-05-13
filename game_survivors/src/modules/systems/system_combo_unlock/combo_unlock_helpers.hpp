#pragma once

#include "combo_unlock_components.hpp"
#include "entt/entity/fwd.hpp"
#include <entt/fwd.hpp>

namespace game2d {

ComboUnlockComponent
generate_combo_component(entt::registry& r);

} // namespace game2d