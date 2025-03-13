#pragma once

#include <entt/fwd.hpp>

namespace game2d {

// declared in header as vfx uses this number as well
const float explosion_radius_pixels = 50;

void
add_explode_on_death_callback(entt::registry& r,
                              entt::entity e,
                              const std::function<bool(entt::registry&, entt::entity)>& cond);

} // namespace game2d