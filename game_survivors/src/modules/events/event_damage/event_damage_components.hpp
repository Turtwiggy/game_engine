#pragma once

#include "modules/systems/system_traits/trait_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

enum class DamageType
{
  PHYSICAL, // subtract armour
  PURE,     // damage not blocked by armour i.e. bleed
};

struct DamageEvent
{
  entt::entity from = entt::null;
  entt::entity to = entt::null;
  float amount = 0.0f;
  DamageType type = DamageType::PHYSICAL;
};

} // namespace game2d