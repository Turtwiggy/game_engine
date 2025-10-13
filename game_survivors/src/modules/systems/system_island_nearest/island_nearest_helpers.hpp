#pragma once

#include "modules/actors/actor_rock/rock_components.hpp"
#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

void
land_player_on_island(entt::registry& r,
                      DebugContoursComponent& island_c,
                      const glm::ivec2 gp,
                      entt::entity boat_e,
                      entt::entity island_e);

std::vector<glm::ivec2>
get_unoccupied_tiles(const DebugContoursComponent& island_c);

std::vector<glm::ivec2>
get_unoccupied_edge_tiles(const DebugContoursComponent& island_c);

bool
occupied(entt::registry& r, const DebugContoursComponent& island_c, glm::ivec2 gp);

entt::entity
e_at_xy(entt::registry& r, const DebugContoursComponent& island_c, glm::ivec2 gp);

void
remove_hidden_state_from_island(entt::registry& r, entt::entity island_e);

} // namespace game2d