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

struct SINGLE_UIBoardShip
{
  bool show = false;
};

} // namespace game2d