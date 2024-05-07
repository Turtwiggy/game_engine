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

// struct Dungeon
// {
//   int width = 50;
//   int height = 50;
//   int seed = 0;
//   int floor = 0;
//   std::vector<Room> rooms;
//   std::vector<StaticDungeonEntity> walls_and_floors;
//   std::vector<std::pair<entt::entity, glm::ivec2>> occupied;
// };

} // namespace game2d