#pragma once

#include "game/components/app.hpp"
#include "game/modules/ai/components.hpp"

#include <entt/entt.hpp>

#include <utility>
#include <vector>

namespace game2d {

std::pair<int, int>
next_dir_along_path(const std::vector<vec2i>& path);

void
pathfind_unit_to_mouse_position(GameEditor& editor, Game& game, const entt::entity& e);

void
shoot(GameEditor& editor, Game& game, const entt::entity& e);

} // namespace game2d