#pragma once

#include "maths/maths.hpp"
#include "modules/gen_dungeons/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
set_generated_entity_positions(entt::registry& r, DungeonGenerationResults& results, engine::RandomState& rnd);

} // namespace game2d