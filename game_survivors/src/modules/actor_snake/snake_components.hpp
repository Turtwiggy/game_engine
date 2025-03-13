#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

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

  int snake_segments = 8;
  glm::vec2 snake_segment_size = glm::vec2{ 64, 64 };
  float distance_betwee_segment_pixels = snake_segment_size.x;

  float snake_time_to_swap_points = 30;
  float snake_timer = 0.0f;
};

} // namespace game2d