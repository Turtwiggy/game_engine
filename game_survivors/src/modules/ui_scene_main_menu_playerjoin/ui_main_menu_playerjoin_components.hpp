#pragma once

#include <entt/entt.hpp>
#include <steam/isteaminput.h>

namespace game2d {

struct SINGLE_SteamControllerGameState
{
  int players = 4;
  std::vector<InputHandle_t> handles;

  SINGLE_SteamControllerGameState() { handles.resize(players, 0); };
};

} // namespace game2d