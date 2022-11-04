#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <utility>
#include <vector>

namespace game2d {

// e.g. walls and floors
struct StaticDungeonEntity
{
  int x = -1;
  int y = -1;
  int room_index = -1;
  entt::entity entity = entt::null;
};

struct Room
{
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
  int w = 0;
  int h = 0;
};

struct Dungeon
{
  int width = 50;
  int height = 50;
  int seed = 0;
  int floor = 0;

  std::vector<Room> rooms;
  std::vector<StaticDungeonEntity> walls_and_floors;

  // note: only accurate at generation stage, not after
  std::vector<std::pair<entt::entity, glm::ivec2>> occupied;
};

enum class GridDirection : size_t
{
  north,
  south,
  east,
  west,

  // allow diagonals?
  // north_east,
  // south_east,
  // south_west,
  // north_west,

  count,
};

} // namespace game2d