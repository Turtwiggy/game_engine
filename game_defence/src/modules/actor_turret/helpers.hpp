#pragma once

#include "actors.hpp"
#include "components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

[[nodiscard]] ClosestInfo
get_closest(entt::registry& r, const entt::entity& e, const std::vector<EntityType>& types);

} // namespace game2d