#pragma once

#include "helpers/line.hpp"

#include <entt/entt.hpp>
#include <functional> // for std::hash
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace game2d {

struct RoomAABB
{
  glm::ivec2 center{ 0, 0 };
  glm::ivec2 size{ 0, 0 };
};

struct RoomAABBHash
{
  size_t operator()(const RoomAABB& room) const
  {
    std::size_t h1 = std::hash<glm::ivec2>{}(room.center);
    std::size_t h2 = std::hash<glm::ivec2>{}(room.size);
    return h1 ^ (h2 << 1); // or use boost::hash_combine
  };
};

struct Room
{
  glm::ivec2 tl{ 0, 0 };
  RoomAABB aabb; // aabb is in tile-space, not world-space.
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
inline bool
operator==(const Tunnel& a, const Tunnel& b)
{
  return a.line_0 == b.line_0 && a.line_1 == b.line_1;
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

  std::vector<entt::entity> floor_tiles;
};

struct FloorComponent
{
  bool placeholder = true;
};

} // namespace game2d