#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_AlivePlayers
{
  std::vector<entt::entity> players;

  static SINGLE_AlivePlayers instance;
};

} // namespace game2d