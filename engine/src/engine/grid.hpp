#pragma once

// c++ lib headers
#include <iostream>
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
//  N  0 | 0  1  2  3
//  B  1 |    4  5  6
//  P  2 |       7  8
//  E  3 |          9
//       |-----------
//         0  1  2  3
//         N  B  P  E

// the get_layer_value is:
// x=2... y=1... x-max=4...
// layer_value: (4 - (2-1)) + (4 -(1-1)) = 7
// final_value: 7 + 2 - 2
// because 9 is the first element on the 2nd y layer.
template<typename T>
[[nodiscard]] inline T
get_cell_mirrored_grid(const std::vector<T>& t, int x, int y, int x_max)
{
  // make sure the X value is always the greatest value
  if (x < y) {
    int temp = x;
    x = y;
    y = temp;
  }
  // after sorting, if x is still less than y,
  // this is invalid grid entry
  if (x < y) {
    std::cerr << " INVALID GRID ENTRY x:" << x << " y:" << y << std::endl;
    exit(1); // harsh, but I'd prefer not try and recover currently
  }

  int layer_value = get_layer_value(y, x_max);
  int v = layer_value + x - y;
  return t[v];
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
convert_grid_space_to_worldspace(glm::ivec2 pos, int grid_size)
{
  return glm::vec2{ pos.x, pos.y } * static_cast<float>(grid_size);
}

} // namespace grid

} // namespace game2d