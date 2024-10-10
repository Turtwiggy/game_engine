#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

struct MapInfo
{
  int idx_in_map = 0;

  // each gridcell can have multiple entries
  // idx_in_map_tile is the index in the std::vector<entity>
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

} // namespace game2d