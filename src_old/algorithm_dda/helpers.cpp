#include "helpers.hpp"

#include "engine/maths/maths.hpp"

namespace game2d {

DDAOutput
dda_raycast(const DDAInput& input, const std::vector<int>& map)
{
  const auto& start = input.start; // player
  const auto& end = input.end;     // cursor
  const auto& map_size = input.map_size;

  const glm::vec2 dir = engine::normalize_safe(end - start);

  const glm::vec2 unit_step_size = {
    // clang-format off

    // how long is the ray for one x-unit?
    sqrt(1 + (dir.y / dir.x) * (dir.y / dir.x)),

    // how long is the ray for one y-unit?
    sqrt(1 + (dir.x / dir.y) * (dir.x / dir.y))

    // clang-format on
  };

  glm::ivec2 map_check = start;

  // whichever ray is the shortest,
  // is the axis i walk along in orer to check the next ray
  // if x-axis is shorter, we go in the x-axis

  glm::vec2 ray_length;
  glm::ivec2 step;

  if (dir.x < 0) {
    step.x = -1;
    ray_length.x = (start.x - float(map_check.x)) * unit_step_size.x;
  } else {
    step.x = 1;
    ray_length.x = (float(map_check.x + 1) - start.x) * unit_step_size.x;
  }

  if (dir.y < 0) {
    step.y = -1; // up
    ray_length.y = (start.y - float(map_check.y)) * unit_step_size.y;
  } else {
    step.y = 1; // down
    ray_length.y = (float(map_check.y + 1) - start.y) * unit_step_size.y;
  }

  const float max_distance = 100.0f;
  bool tile_found = false;
  float distance = 0.0f;
  while (!tile_found && distance < max_distance) {

    // exit conditions
    if (map_check.x > input.map_size.x || map_check.x < 0 || map_check.y > input.map_size.y || map_check.y < 0)
      break;

    // Walk
    if (ray_length.x < ray_length.y) {
      map_check.x += step.x;
      distance = ray_length.x;
      ray_length.x += unit_step_size.x;
    }
    //
    else {
      map_check.y += step.y;
      distance = ray_length.y;
      ray_length.y += unit_step_size.y;
    }

    if (map_check.x >= 0 && map_check.x < map_size.x && map_check.y >= 0 && map_check.y < map_size.y) {
      if (map[map_check.y * map_size.x + map_check.x] == 1) {
        tile_found = true;
      }
    }
  }

  glm::vec2 intersection{ 0, 0 };
  if (tile_found)
    intersection = start + dir * distance;

  DDAOutput output;
  output.has_intersection = false;
  output.distance = distance;
  output.intersection = intersection;
  return output;
}

} // namespace game2d