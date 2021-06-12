#pragma once

// c++ lib headers
#include <vector>

// other lib headers
#include <glm/glm.hpp>

namespace game2d {

namespace grid {

[[nodiscard]] inline int
get_layer_value(int y, int x_max)
{
  if (y <= 0)
    return 0;
  return get_layer_value(y - 1, x_max) + (x_max - (y - 1));
}

// template<typename T>
// [[nodiscard]] inline std::vector<T>
// create_empty_grid(std::vector<T>& vec, int x, int y)
// {
//   T t;
//   for (int i = 0; i < x * y; i++) {
//     vec.push_back(t);
//   }
// }

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

// e.g.
// a grid shaped
// ~~~~~~~~~~~~~
// 0  1  2  3  4
// 5  6  7  8
// 9  10 11
// 12 13
// 14
// ~~~~~~~~~~~~~~
// the get_layer_value is:
// y=2... x-max=5...
// (5 - (2-1)) + (5 -(1-1)) = 9
// because 9 is the first element on the 2nd y layer.
template<typename T>
[[nodiscard]] inline T
get_cell_mirrored_grid(const std::vector<T>& t, int x, int y, int x_max)
{
  if (y < x) {
    int temp = x;
    x = y;
    y = temp;
  }

  int val = get_layer_value(y, x_max) - x + y;
  return t[val];
}

[[nodiscard]] inline glm::ivec2
convert_world_space_to_grid_space(const glm::vec2& world_space, int grid_size)
{
  float x = world_space.x;
  float y = world_space.y;

  int grid_x = x < 0.0f ? static_cast<int>((x - grid_size) / grid_size) : static_cast<int>(x / grid_size);
  int grid_y = y < 0.0f ? static_cast<int>((y - grid_size) / grid_size) : static_cast<int>(y / grid_size);

  return glm::ivec2{ grid_x, grid_y };
}

[[nodiscard]] inline glm::vec2
convert_grid_space_to_worldspace(glm::ivec2 pos, float grid_size)
{
  return pos * grid_size;
}

} // namespace grid

} // namespace game2d