#pragma once

#include "actors.hpp"
#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

[[nodiscard]] ClosestInfo
get_closest(entt::registry& r, const entt::entity& e, const EntityType& type);

} // namespace game2d