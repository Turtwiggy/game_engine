#pragma once

#include "maths/maths.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/selected_interactions/components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

std::vector<Room>
create_all_rooms(DungeonGenerationCriteria& data,
                 const MapComponent& map,
                 std::vector<int>& wall_or_floors,
                 engine::RandomState& rnd);

void
generate_dungeon(entt::registry& r);

} // namespace game2d