#pragma once

#include "maths/maths.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

DungeonGenerationResults
generate_rooms(entt::registry& r, const DungeonGenerationCriteria& data, engine::RandomState& rnd);

void
instantiate_tunnels(entt::registry& r, const DungeonGenerationResults& results);

void
instantiate_walls(entt::registry& r, const DungeonGenerationResults& results);

void
set_generated_entity_positions(entt::registry& r, const DungeonGenerationResults& results, engine::RandomState& rnd);

void
set_player_positions(entt::registry& r, const DungeonGenerationResults& results, engine::RandomState& rnd);

// helper
entt::entity
create_wall(entt::registry& r, const glm::ivec2& pos, const glm::ivec2& size, const entt::entity& parent);

// EntityType
// generate_monster();

// EntityType
// generate_item();

} // namespace game2d