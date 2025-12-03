#pragma once

#include "modules/actors/actor_enemy/components.hpp"
#include "modules/systems/system_particles/components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
add_explode_on_death_callback(entt::registry& r,
                              entt::entity e,
                              const float explosion_radius_pixels,
                              const std::function<bool(entt::registry&, entt::entity)>& cond,
                              const ParticleType particle_type);

} // namespace game2d