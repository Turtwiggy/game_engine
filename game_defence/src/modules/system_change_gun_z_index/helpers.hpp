#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
set_z_index(entt::registry& r, const entt::entity e, const int index);

} // namespace game2d