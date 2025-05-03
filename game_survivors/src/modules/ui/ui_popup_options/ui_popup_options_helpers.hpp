#pragma once

#include "modules/core/options/options_components.hpp"
#include "ui_popup_options_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
back_to_main_menu(entt::registry& r, SINGLE_OptionsMenuState& ui_c);

std::shared_ptr<IOption>
get_option(entt::registry& r, const GAME_OPTIONS o);

} // namespace game2d