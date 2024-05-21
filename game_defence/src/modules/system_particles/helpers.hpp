#pragma once

#include "modules/system_particles/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

entt::entity
create_particle(entt::registry& r, const ParticleDescription& desc);

} // namespace game2d