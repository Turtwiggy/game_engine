#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

struct PathfindComponent
{
  int cost = 1;
};

struct vec2i
{
  int x = 0;
  int y = 0;

  vec2i() = default;
  vec2i(const int& ox, const int& oy)
    : x(ox)
    , y(oy){};
  vec2i(const glm::ivec2& xy)
    : x(xy.x)
    , y(xy.y){};

  // spaceship operator
  auto operator<=>(const vec2i&) const = default;
};

struct astar_cell
{
  int idx = 0;
  vec2i pos;
  int distance = INT_MAX;
  int cost = 1;
};

struct GeneratedPathComponent
{
  glm::vec2 src_pos{ 0, 0 };
  glm::vec2 dst_pos{ 0, 0 };
  entt::entity dst_ent = entt::null;
  std::vector<glm::ivec2> path;

  // better for avoiding corners with obstacles
  bool required_to_clear_path = false;
  std::vector<bool> path_cleared;

  // bool aim_for_exact_position = false;

  bool wait_at_destination = true;
  float wait_time_ms = 75;
  float wait_time_ms_left = 0; // move immediately
};

} // namespace game2d