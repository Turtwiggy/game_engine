#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
drop_items_on_death_callback(entt::registry& r, const entt::entity e);

} // namespace game2d