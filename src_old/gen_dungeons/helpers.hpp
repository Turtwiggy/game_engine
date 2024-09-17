#pragma once

#include "engine/maths/maths.hpp"
#include "helpers/line.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"


#include <entt/entt.hpp>

#include <vector>

namespace game2d {

DungeonGenerationResults
generate_rooms(entt::registry& r, const DungeonGenerationCriteria& data, engine::RandomState& rnd);

void
connect_rooms_via_nearest_neighbour(entt::registry& r, DungeonGenerationResults& result);

//

std::vector<entt::entity>
inside_room(entt::registry& r, const glm::ivec2& gridpos);

//

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonGenerationResults& result);

void
instantiate_edges(entt::registry& r, MapComponent& map);

void
instantiate_doors(entt::registry& r, MapComponent& map);

} // namespace game2d