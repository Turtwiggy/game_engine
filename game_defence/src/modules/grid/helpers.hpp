#pragma once

#include "modules/grid/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

void
move_entity_on_map(entt::registry& r, const entt::entity src_e, const glm::ivec2& worldspace_pos);

int
get_lowest_cost_neighbour(entt::registry& r, const MapComponent& map, const int src_idx, const entt::entity e);

std::vector<glm::ivec2>
generate_path(entt::registry& r, const entt::entity src_e, const glm::ivec2& worldspace_pos, const int limit);

void
update_path_to_tile_next_to_player(entt::registry& r, const entt::entity src_e, const entt::entity dst_e, const int limit);

void
update_entity_path(entt::registry& r, const entt::entity src_e, const std::vector<glm::ivec2>& path);

} // namespace game2d