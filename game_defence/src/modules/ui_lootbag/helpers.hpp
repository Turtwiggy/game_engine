#pragma once

#include "modules/ui_inventory/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

bool
inv_is_empty(entt::registry& r, const DefaultInventory& inv);

} // namespace game2d