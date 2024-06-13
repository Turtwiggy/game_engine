#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

// Also attached is a transform component
struct SpaceshipComponent
{
  bool placeholder = true;
};

struct SpaceshipRoomComponent
{
  // std::vector<entt::entity> corners;
  // std::vector<entt::entity> doors;
  // float biome_type;
  // float oxygen_amount;

  bool placeholder = true;
};

// enum class SpaceshipSystem
// {
//   OXYGEN,
//   ENGINES,
//   WEAPONS,
//   SHIELDS,
//   MEDBAY,
//   DOORS,
//   CAMERAS,
//   PILOTING,
// };

// Walls are basically "Lines"
struct Wall
{
  // order matters for these
  glm::ivec2 p0{ 0, 0 };
  glm::ivec2 p1{ 0, 0 };

  entt::entity parent_room = entt::null;

  std::vector<glm::ivec2> intersections; // storage for when this wall is being intersected
  std::vector<entt::entity> debug_intersections;
};

} // namespace game2d