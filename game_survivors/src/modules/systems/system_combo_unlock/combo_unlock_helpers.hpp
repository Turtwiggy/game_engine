#pragma once

#include "combo_unlock_components.hpp"

#include "entt/entity/fwd.hpp"
#include <entt/fwd.hpp>

namespace game2d {

ComboUnlockComponent
generate_combo_component(entt::registry& r);

std::string
get_sprite_for_combodir(COMBO_DIR dir);

} // namespace game2d