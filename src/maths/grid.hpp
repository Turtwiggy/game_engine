#pragma once

// other lib headers
#include <glm/glm.hpp>

// c++ lib headers
#include <vector>

namespace engine {

namespace grid {

[[nodiscard]] inline glm::vec2
grid_space_to_world_space(glm::ivec2 pos, int grid_size)
{
  return glm::vec2{ pos.x, pos.y } * static_cast<float>(grid_size);
}

[[nodiscard]] inline glm::ivec2
world_space_to_grid_space(const glm::vec2& pos, int grid_size)
{
  int grid_x = static_cast<int>(pos.x / grid_size);
  int grid_y = static_cast<int>(pos.y / grid_size);
  return glm::ivec2{ grid_x, grid_y };
}

inline int
grid_position_to_index(const glm::ivec2& pos, const int x_max)
{
  return x_max * pos.y + pos.x;
}

inline glm::ivec2
index_to_grid_position(const int index, const int x_max, const int y_max)
{
  const int x = index % x_max;
  const int y = static_cast<int>(index / static_cast<float>(y_max));
  return { x, y };
}

// a grid shaped
// ~~~~~~~~~~~~~
// 0  1  2  3  4
// 5  6  7  8  9
// 10 11 12 13 14
// 15 16 17 18 19
// 20 21 22 23 24
// ~~~~~~~~~~~~~~
template<typename T>
[[nodiscard]] inline T&
get_cell(const std::vector<T>& t, int x, int y, int x_max)
{
  return t[x_max * y + x];
}

[[nodiscard]] inline glm::ivec2
world_space_to_clamped_world_space(const glm::vec2& world_space, int grid_size)
{
  float x = world_space.x;
  float y = world_space.y;
  int grid_x = x < 0.0f ? static_cast<int>((x - grid_size) / grid_size) : static_cast<int>(x / grid_size);
  int grid_y = y < 0.0f ? static_cast<int>((y - grid_size) / grid_size) : static_cast<int>(y / grid_size);
  return glm::ivec2{ grid_size * grid_x, grid_size * grid_y };
}

} // namespace grid

} // namespace engine