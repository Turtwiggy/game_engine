#pragma once

#include "engine/app/application.hpp"
#include "game/components/app.hpp"

namespace game2d {

void
update_ui_main_menu_system(engine::SINGLETON_Application& app, GameEditor& editor, Game& game, uint32_t& seed);

} // namespace game2d