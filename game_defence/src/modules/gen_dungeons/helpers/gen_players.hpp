#pragma once

#include "maths/maths.hpp"
#include "modules/gen_dungeons/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

// TODO: replace with the number of player's crew
// TODO: check if occupied
void
set_player_positions(entt::registry& r, const DungeonGenerationResults& results, engine::RandomState& rnd);

} // namespace game2d