#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct RequestShowBoardShipButton
{
  bool request = false;
};

struct RequestSpawnCargoboxes
{
  int amount = 0;
};

} // namespace game2d