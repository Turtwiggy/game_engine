#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

struct BowComponent
{
  bool in_windup = false;
  float bow_bullet_speed = 100.0f;
  std::vector<entt::entity> arrows;
};

struct WantsToShoot
{
  //
};

} // namespace game2d