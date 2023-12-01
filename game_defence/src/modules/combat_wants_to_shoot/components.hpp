#pragma once

namespace game2d {

struct WantsToShoot
{
  bool placeholder = true;
};

struct WantsToReleaseShot
{
  bool placehoolder = true;
};

struct TargetComponent
{
  entt::entity target = entt::null;
};

} // namespace game2d