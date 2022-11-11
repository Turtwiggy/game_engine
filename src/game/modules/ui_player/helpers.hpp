#pragma once

#include "game/components/app.hpp"

#include <entt/entt.hpp>
#include <imgui.h>

#include <array>

namespace game2d {

std::array<ImVec2, 2>
convert_sprite_to_uv(GameEditor& editor, Game& game, int x, int y);

void
draw_healthbar(Game& game, const entt::entity& entity);

} // namespace game2d