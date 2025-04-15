#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct WeaponLevelReachedEvent
{
  int level = 0;
  entt::entity par_e = entt::null;
  entt::entity wep_e = entt::null;
};

} // namespace game2d