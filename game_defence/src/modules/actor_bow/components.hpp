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

} // namespace game2d