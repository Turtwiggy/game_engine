#pragma once

// other lib headers
#include <glm/glm.hpp>

// c++ lib headers
#include <vector>

namespace engine {

namespace grid {

enum class GridDirection : size_t
{
  north,
  south,
  east,
  west,

  // diagonals
  north_east,
  south_east,
  north_west,
  south_west,
};

// e.g. 0, 0 should return {1, 0}, {1, 1}, {0, 1}
[[nodiscard]] std::vector<std::pair<GridDirection, int>>
get_neighbour_indicies(const int x, const int y, const int x_max, const int y_max);

std::vector<std::pair<GridDirection, glm::ivec2>>
get_neighbour_gridpos(const glm::ivec2& xy, const int x_max, const int y_max);

[[nodiscard]] std::vector<std::pair<GridDirection, int>>
get_neighbour_indicies_with_diagonals(const int x, const int y, const int x_max, const int y_max);

[[nodiscard]] inline glm::vec2
grid_space_to_world_space(const glm::ivec2 pos, const int grid_size)
{
  return glm::vec2{ pos.x, pos.y } * static_cast<float>(grid_size);
};

[[nodiscard]] inline glm::ivec2
worldspace_to_grid_space(const glm::vec2& pos, const int grid_size)
{
  const int grid_x = static_cast<int>(glm::abs(pos.x) / static_cast<float>(grid_size));
  const int grid_y = static_cast<int>(glm::abs(pos.y) / static_cast<float>(grid_size));
  if (pos.x >= 0 && pos.y >= 0)
    return { grid_x, grid_y };

  // Brain Hurty.
  // But basically, if x (or y) was < 0,
  // It was the case that it was incorrectly returning
  // e.g. {-10, 0} with a gridsize 32 as {0, 0} where it should've been {-1, 0}.
  // The below shifts the negative grid scale by 1 in the direction if that
  // pos.x or pos.y is less than 0.
  int final_grid_x = grid_x;
  int final_grid_y = grid_y;
  if (pos.x < 0)
    final_grid_x = -(grid_x + 1);
  if (pos.y < 0)
    final_grid_y = -(grid_y + 1);
  return { final_grid_x, final_grid_y };
};

[[nodiscard]] inline glm::ivec2
worldspace_to_clamped_gridspace(const glm::ivec2 pos, const int grid_size, const int xmax, const int ymax)
{
  auto gridpos = engine::grid::worldspace_to_grid_space(pos, grid_size);
  gridpos.x = glm::clamp(gridpos.x, 0, xmax - 1);
  gridpos.y = glm::clamp(gridpos.y, 0, ymax - 1);
  return gridpos;
};

inline int
grid_position_to_index(const glm::ivec2& pos, const int x_max)
{
  return x_max * pos.y + pos.x;
};

[[nodiscard]] inline int
worldspace_to_index(const glm::ivec2& pos, const int grid_size, const int xmax, const int ymax)
{
  const auto gridpos = worldspace_to_clamped_gridspace(pos, grid_size, xmax, ymax);
  return engine::grid::grid_position_to_index(gridpos, xmax);
};

constexpr int
grid_position_to_clamped_index(const glm::ivec2& pos, const int xmax, const int ymax)
{
  const auto idx = xmax * pos.y + pos.x;
  if (idx < 0)
    return 0;
  if (idx >= (xmax * ymax))
    return (xmax * ymax) - 1;
  return idx;
};

glm::ivec2
index_to_grid_position(const int index, const int x_max, const int y_max);

inline glm::vec2
index_to_world_position(const int index, const int x_max, const int y_max, const int size)
{
  return index_to_grid_position(index, x_max, y_max) * size;
};

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
};

[[nodiscard]] inline glm::vec2
worldspace_to_clamped_world_space(const glm::vec2& world_space, int grid_size)
{
  const float x = world_space.x;
  const float y = world_space.y;
  const int grid_x = x < 0.0f ? static_cast<int>((x - grid_size) / grid_size) : static_cast<int>(x / grid_size);
  const int grid_y = y < 0.0f ? static_cast<int>((y - grid_size) / grid_size) : static_cast<int>(y / grid_size);
  return glm::ivec2{ grid_size * grid_x, grid_size * grid_y };
};

[[nodiscard]] inline std::vector<glm::ivec2>
get_grid_cells(const glm::vec2& pos_tl, const glm::vec2& size, const int tilesize)
{
  std::vector<glm::ivec2> cells;
  for (float y = pos_tl.y; y <= pos_tl.y + size.y; y += tilesize) {
    for (float x = pos_tl.x; x <= pos_tl.x + size.x; x += tilesize) {
      const auto worldspace_pos = glm::vec2{ x, y };
      const auto gridspace_pos = engine::grid::worldspace_to_grid_space(worldspace_pos, tilesize);
      cells.push_back((gridspace_pos));
    }
  }
  return cells;
};

// e.g. {0, 0}, {1, 0} would return East
// engine::grid::GridDirection
// which_quadrant_is_b(const glm::ivec2& a, const glm::ivec2& b);

// // e.g. {1, 0} would return true for East
// bool
// dir_is_in_quadrant(const glm::ivec2& dir, const engine::grid::GridDirection& direction);

} // namespace grid

} // namespace engine