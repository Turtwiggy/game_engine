#pragma once

#include "event_upgrade_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
handle_upgrade_event(entt::registry& r, const UpgradeEvent& evt);

} // namespace game2d