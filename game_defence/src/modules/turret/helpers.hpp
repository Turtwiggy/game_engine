#pragma once

#include "actors.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

struct ClosestInfo
{
  entt::entity e = entt::null;
  int distance2 = std::numeric_limits<int>::max();
};

[[nodiscard]] ClosestInfo
get_closest(const entt::registry& r, const entt::entity& e, const EntityType& type);

} // namespace game2d