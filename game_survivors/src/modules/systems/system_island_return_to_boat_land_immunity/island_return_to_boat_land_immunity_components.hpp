#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct IslandCollisionImmunity
{
  float immunity_time_Left = 2.0f;
  entt::entity island_e = entt::null; // you can land on different islands
};

} // namespace game2d