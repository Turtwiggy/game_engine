#pragma once

#include "modules/events/event_shoot/event_shoot_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
update_muzzleflash_system(entt::registry& r);

void
handle_shoot_event__muzzleflash(entt::registry& r, const ShootEvent& evt);

} // namespace game2d