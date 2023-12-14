#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct WantsToShoot
{
  bool placeholder = true;
};

struct WantsToReleaseShot
{
  bool placehoolder = true;
};

// a moving target
struct DynamicTargetComponent
{
  entt::entity target = entt::null;
};

} // namespace game2d