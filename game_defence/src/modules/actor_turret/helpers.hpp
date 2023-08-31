#pragma once

#include "actors.hpp"
#include "components.hpp"
#include "modules/combat_damage/components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

[[nodiscard]] entt::entity
get_closest(entt::registry& r, const entt::entity& e);

} // namespace game2d