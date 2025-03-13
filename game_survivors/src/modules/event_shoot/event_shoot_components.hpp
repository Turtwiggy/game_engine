#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct ShootEvent
{
  entt::entity parent_e = entt::null;
  entt::entity weapon_e = entt::null;
};

} // namespace game2d