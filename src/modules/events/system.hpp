#pragma once

#include "engine/app/application.hpp"
#include "game/components/app.hpp"

namespace game2d {

void
init_input_system(Game& g);

void
update_input_system(engine::SINGLETON_Application& app, const GameEditor& e, Game& g);

} // namespace game2d