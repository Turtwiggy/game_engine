#pragma once

#include "modules/grid/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

struct MapInfo
{
  int idx_in_map = 0;
  int idx_in_map_tile = 0;
};

void
add_entity_to_map(entt::registry& r, const entt::entity src_e, const int idx);

// returns std::nullopt if entity not in map
std::optional<MapInfo>
get_entity_mapinfo(entt::registry& r, entt::entity e);

bool
move_entity_on_map(entt::registry& r, const entt::entity src_e, const int idx);

void
remove_entity_from_map(entt::registry& r, const MapInfo& info);

//

std::vector<glm::ivec2>
generate_path(entt::registry& r, int src_idx, int dst_idx, const size_t limit);

} // namespace game2d