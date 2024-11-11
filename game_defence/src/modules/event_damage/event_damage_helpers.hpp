#pragma once

#include "modules/raws/raws_components.hpp"
#include <entt/entt.hpp>

namespace game2d {

enum class DamageType
{
  PHYSICAL, // subtract armour
  PURE,     // damage not blocked by armour
};

struct DamageEvent
{
  // entt::entity from;
  entt::entity to = entt::null;
  int amount = 0;
  DamageType type = DamageType::PHYSICAL;
  std::vector<Trait> traits;
};

int
calculate_damage_to_take(entt::registry& r, entt::entity e, int amount, const DamageType& type);

void
handle_damage_event(entt::registry& r, const DamageEvent& evt);

} // namespace game2d