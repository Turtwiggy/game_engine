#pragma once

#include "modules/core/options/options_components.hpp"
#include "ui_popup_options_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

std::shared_ptr<IOption>
get_option(entt::registry& r, const GAME_OPTIONS o);

} // namespace game2d