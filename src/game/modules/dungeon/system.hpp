#pragma once

#include "game/components/app.hpp"

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
generate_dungeon(GameEditor& editor, entt::registry& r, const Dungeon& d, uint32_t seed);

void
update_dungeon_system(GameEditor& editor, Game& game);

} // namespace game2d