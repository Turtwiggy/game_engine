#pragma once

#include "app/application.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_ui_main_menu_system(engine::SINGLETON_Application& app, entt::registry& r);

} // namespace game2d