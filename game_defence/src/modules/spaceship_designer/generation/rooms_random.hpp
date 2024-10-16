#pragma once

#include "components.hpp"
#include "engine/maths/maths.hpp"
#include "modules/map/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

DungeonIntermediate
generate_rooms(entt::registry& r, const DungeonGenerationCriteria& data, engine::RandomState& rnd);

void
connect_rooms_via_nearest_neighbour(entt::registry& r, DungeonIntermediate& result);

void
convert_tunnels_to_rooms(entt::registry& r, const DungeonIntermediate& results);

std::vector<entt::entity>
inside_room(entt::registry& r, const glm::ivec2& gridpos);

std::vector<int>
get_empty_slots_in_map(entt::registry& r, const MapComponent& map_c);

std::vector<int>
get_empty_slots_in_room(entt::registry& r, const MapComponent& map_c, const Room& room);

} // namespace game2d