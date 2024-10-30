#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct InitiativeComponent
{
  int initiative = 1; // lower initiative, go quicker
};

struct SINGLE_Initiative
{
  std::vector<entt::entity> order;
};

} // namespace game2d