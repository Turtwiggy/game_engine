#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>
#include <vector>

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

struct RoomName
{
  std::string name = "Unknown";
};

inline bool
operator==(const Room& a, const Room& b)
{
  return a.tiles_idx == b.tiles_idx;
};

struct DungeonGenerationCriteria
{
  int max_rooms = 300;
  int room_size_min = 4;
  int room_size_max = 10;
};

enum class FloorType
{
  FLOOR = 0,
  WALL = 1,
  AIRLOCK = 2,
};

struct DungeonIntermediate
{
  std::vector<Room> rooms;
  std::vector<FloorType> floor_types;
};

struct RequestGenerateDungeonComponent
{
  bool placeholder = true;
};

} // namespace game2d