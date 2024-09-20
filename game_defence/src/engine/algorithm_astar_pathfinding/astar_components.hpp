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
  vec2i grid_pos{ 0, 0 }; // can be negative. e.g. {-1, -1} is valid
  int distance = INT_MAX;
  int cost = 1;
};

struct GeneratedPathComponent
{
  glm::vec2 src_pos{ 0, 0 };
  glm::vec2 dst_pos{ 0, 0 };
  entt::entity dst_ent = entt::null;
  std::vector<glm::ivec2> path;
};

} // namespace game2d