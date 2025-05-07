#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
add_explode_on_death_callback(entt::registry& r,
                              entt::entity e,
                              const float explosion_radius_pixels,
                              const std::function<bool(entt::registry&, entt::entity)>& cond,
                              const std::string key);

} // namespace game2d