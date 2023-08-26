#pragma once

#include <entt/entt.hpp>

#include <glm/glm.hpp>

namespace game2d {

struct ArrowComponent
{
  bool placeholder = true;
};

struct BowComponent
{
  bool in_windup = false;
  float offset = 30.0f;

  float lerp_speed = 45.0f;
  float lerp_amount = 0.0f; // 0-1
  glm::ivec2 target_position{ 0, 0 };

  float time_between_bullets = 0.10f;
  float time_between_bullets_left = 0.0f;
  // int bullets_in_clip = 6;
  // int bullets_in_clip_left = 6;
  // float time_between_reloads = 0.25f;
  // float time_between_reloads_left = 0.25f;

  std::vector<entt::entity> my_arrows;
};

}; // namespace game2d