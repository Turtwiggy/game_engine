#pragma once

#include "physics/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

// struct map_builder_interface
// {
//   virtual void build();

//   // Given the map generation algoritm, generate some points
//   // that could be valid for the map type for the player to spawn
//   virtual std::vector<glm::ivec2> generate_starting_positions(int amount);
// };

struct Room
{
  glm::ivec2 tl{ 0, 0 };
  AABB aabb;
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
};

} // namespace game2d