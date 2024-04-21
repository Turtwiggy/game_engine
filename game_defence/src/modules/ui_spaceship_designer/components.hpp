#pragma once

#include <entt/entt.hpp>

namespace game2d {

// Also attached is a transform component
struct SpaceshipComponent
{
  bool placeholder = true;
};

struct SpaceshipPointComponent
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

} // namespace game2d