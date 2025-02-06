#pragma once

#include "engine/app/application.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_ui_popup_pause_system(engine::SINGLE_Application& app, entt::registry& r);

} // namespace game2d