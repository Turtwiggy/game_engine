#pragma once

#include <entt/entt.hpp>

namespace game2d {

// when added, start counting down the deaththroes component
struct IsDyingComponent
{
  bool placeholder = true;
};

// The unit is in the process of dying.
struct DeathThroesComponent
{
  // the thing that probably killed you
  bool evt_from_set = false;
  entt::entity evt_from = entt::null;

  float death_throws_time_max = 1.5f;
  float death_throws_time_cur = 1.5f;
};

} // namespace game2d