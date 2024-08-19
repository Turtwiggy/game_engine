#pragma once

#include "helpers/line.hpp"
#include "maths/maths.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

DungeonGenerationResults
generate_rooms(entt::registry& r, const DungeonGenerationCriteria& data, engine::RandomState& rnd);

void
set_generated_entity_positions(entt::registry& r, DungeonGenerationResults& results, engine::RandomState& rnd);

//

std::pair<bool, std::optional<Room>>
inside_room(const MapComponent& map, const std::vector<Room>& rooms, const glm::ivec2& gridpos);

std::vector<Tunnel>
inside_tunnels(const std::vector<Tunnel>& ts, const glm::ivec2& gridpos);

//

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonGenerationResults& result);

void
instantiate_edges(entt::registry& r, MapComponent& map);

// EntityType
// generate_monster();

// EntityType
// generate_item();

} // namespace game2d