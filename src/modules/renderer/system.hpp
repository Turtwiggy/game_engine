#pragma once

#include "engine/app/application.hpp"
#include "game/components/app.hpp"

namespace game2d {

void
init_render_system(const engine::SINGLETON_Application& app, GameEditor& editor);

void
update_render_system(GameEditor& editor, Game& game);

void
end_frame_render_system(entt::registry& registry);

} // namespace game2d
