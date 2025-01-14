#pragma once

#include <entt/entt.hpp>

namespace game2d {

const float explosion_radius = 50;

std::vector<entt::entity>
enemies_in_range(entt::registry& r, entt::entity e, float radius);

void
add_explode_on_death_callback(entt::registry& r, entt::entity e);

} // namespace game2d