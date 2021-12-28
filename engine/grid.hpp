#pragma once

// other lib headers
#include <glm/glm.hpp>

// c++ lib headers
#include <iostream>
#include <vector>

namespace engine {

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
//  0 | 0  1  2  3
//  1 |    4  5  6
//  2 |       7  8
//  3 |          9
//    |-----------
//      0  1  2  3
// the get_layer_value is:
// x=2... y=1... x-max=4...
// layer_value: (4 - (2-1)) + (4 -(1-1)) = 7
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
  }

  int layer_value = get_layer_value(y, x_max);
  int v = layer_value + x - y;
  return t[v];
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

[[nodiscard]] inline glm::ivec2
world_space_to_grid_space(const glm::vec2& pos, int grid_size)
{
  int grid_x = static_cast<int>(pos.x / grid_size);
  int grid_y = static_cast<int>(pos.y / grid_size);
  return glm::ivec2{ grid_x, grid_y };
}

[[nodiscard]] inline glm::vec2
grid_space_to_world_space(glm::ivec2 pos, int grid_size)
{
  return glm::vec2{ pos.x, pos.y } * static_cast<float>(grid_size);
}

// returns the unique (no duplicates) cells an object is in
inline void
get_unique_cells(glm::vec2 pos, glm::vec2 size, int grid_size, std::vector<glm::ivec2>& results)
{
  results.clear();

  glm::vec2 tl = pos;
  glm::vec2 tr = { pos.x + size.x, pos.y };
  glm::vec2 bl = { pos.x, pos.y + size.y };
  glm::vec2 br = { pos.x + size.x, pos.y + size.y };

  // always push tl
  glm::ivec2 gc = grid::world_space_to_grid_space(tl, grid_size);
  results.push_back(gc);

  gc = grid::world_space_to_grid_space(tr, grid_size);
  auto it = std::find_if(
    results.begin(), results.end(), [&gc](const glm::ivec2& obj) { return obj.x == gc.x && obj.y == gc.y; });
  if (it == results.end()) {
    results.push_back(gc);
  }

  gc = grid::world_space_to_grid_space(bl, grid_size);
  it = std::find_if(
    results.begin(), results.end(), [&gc](const glm::ivec2& obj) { return obj.x == gc.x && obj.y == gc.y; });
  if (it == results.end()) {
    results.push_back(gc);
  }

  gc = grid::world_space_to_grid_space(br, grid_size);
  it = std::find_if(
    results.begin(), results.end(), [&gc](const glm::ivec2& obj) { return obj.x == gc.x && obj.y == gc.y; });
  if (it == results.end()) {
    results.push_back(gc);
  }
}

} // namespace grid

} // namespace game2d