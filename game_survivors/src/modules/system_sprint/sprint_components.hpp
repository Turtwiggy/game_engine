#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct ActorStaminaComponent
{
  float max_stamina = 10;
  float cur_stamina = 10;
  float depletion_rate = 1; // per second
};

} // namespace game2d