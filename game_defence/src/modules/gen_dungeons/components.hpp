#pragma once

#include "helpers/line.hpp"
#include "physics/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct Room
{
  glm::ivec2 tl{ 0, 0 };
  AABB aabb; // aabb is in tile-space, not world-space.

  // occupied only accurate at generation stage
  std::vector<glm::ivec2> occupied;
};
inline bool
operator==(const Room& a, const Room& b)
{
  return a.aabb.center == b.aabb.center && a.aabb.size == b.aabb.size;
};

struct Tunnel
{
  bool horizontal_then_vertical = false;
  std::vector<std::pair<int, int>> line_0;
  std::vector<std::pair<int, int>> line_1;

  Room room;
  Room prev_room;
};

struct RequestGenerateDungeonComponent
{
  bool placeholder = true;
};

struct DungeonGenerationCriteria
{
  int max_rooms = 300;
  int room_size_min = 4;
  int room_size_max = 10;
};

struct DungeonGenerationResults
{
  std::vector<Room> rooms;
  std::vector<Tunnel> tunnels;
  std::vector<int> wall_or_floors;
  std::vector<Line> lines_to_instantiate;
};

} // namespace game2d