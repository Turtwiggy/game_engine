#pragma once

#include "modules/system_traits/trait_components.hpp"

#include <entt/entt.hpp>

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
  int amount = 0;
  DamageType type = DamageType::PHYSICAL;
  std::vector<AquirableTrait> traits;
};

} // namespace game2d