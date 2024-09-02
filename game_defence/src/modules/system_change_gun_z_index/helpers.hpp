#pragma once

#include "modules/renderer/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
set_z_index(entt::registry& r, const entt::entity e, const ZLayer& layer);

} // namespace game2d