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

struct DungeonGenerationCriteria
{
  int max_rooms = 30;
  int room_size_min = 6;
  int room_size_max = 10;
};

} // namespace game2d