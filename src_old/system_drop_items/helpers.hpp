#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
drop_inventory_on_death_callback(entt::registry& r, const entt::entity e);

} // namespace game2d