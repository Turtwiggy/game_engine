#pragma once

#include "modules/ui/ui_scene_select_modifiers/select_modifiers_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

std::shared_ptr<IModifierOption>
get_modifier_option(entt::registry& r, const MODIFIER_OPTIONS o);

std::string
modifier_option_enum_to_display_string(entt::registry& r, const MODIFIER_OPTIONS o);

} // namespace game2d