#pragma once

#include "helpers/line.hpp"

#include <entt/entt.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct RoomAABB
{
  glm::ivec2 center{ 0, 0 };
  glm::ivec2 size{ 0, 0 };
};

struct Room
{
  // tl and aabb only applies if the room is square
  std::optional<glm::ivec2> tl;
  std::optional<RoomAABB> aabb; // aabb is in tile-space, not world-space.

  std::vector<int> tiles_idx;
};

inline bool
operator==(const Room& a, const Room& b)
{
  return a.tiles_idx == b.tiles_idx;
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
  std::vector<int> wall_or_floors;
  std::vector<entt::entity> floor_tiles;
};

struct FloorComponent
{
  bool placeholder = true;
};

} // namespace game2d