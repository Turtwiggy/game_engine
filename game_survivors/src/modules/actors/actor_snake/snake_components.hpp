#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct BossComponent
{
  bool placeholder = true;
};

struct SnakeSegment
{
  bool placeholder = true;
};

struct SnakeData
{
  entt::entity target_e_0 = entt::null;
  entt::entity target_e_1 = entt::null;
  // entt::entity target_e_2 = entt::null;

  int snake_segments = 12;
  glm::vec2 snake_segment_size = glm::vec2{ 32, 32 };
  float distance_between_segment_pixels = snake_segment_size.x;

  float snake_time_to_swap_points = 20;
  float snake_timer = 0.0f;
};

} // namespace game2d