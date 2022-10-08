#pragma once

#include "engine/app/application.hpp"
#include "game/components/app.hpp"

namespace game2d {

void
init(engine::SINGLETON_Application& app, GameEditor& editor, Game& game);

void
fixed_update(GameEditor& editor, Game& game, uint64_t milliseconds_dt);

void
update(engine::SINGLETON_Application& app, GameEditor& editor, Game& game, float dt);

} // namespace game2d