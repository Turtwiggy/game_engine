#pragma once

#include <entt/fwd.hpp>

#include "engine/colour/colour.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"

namespace game2d {

void
generate_rocks(entt::registry& r);

// entt::entity
// get_island_eid(entt::registry& r, glm::vec2 pos);

entt::entity
get_center_island_eid(entt::registry& r);

void
generate_island_life__base_island(entt::registry& r);

void
generate_island_life__other_islands(entt::registry& r);

void
spawn_lighthouse(entt::registry& r, DebugContoursComponent& island_c, const glm::ivec2 gridpos);

void
spawn_cannon(entt::registry& r, engine::RandomState& spawn_rnd, entt::entity island_eid);

void
set_players_as_landed(entt::registry& r);

glm::vec2
get_player_spawn_point_around_starting_island(entt::registry& r, int idx);

} // namespace game2d