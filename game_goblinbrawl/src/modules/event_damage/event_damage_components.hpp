#pragma once

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
};

// Wait X ms. If you fire a damage event,
// and your opponent fires a damage event in that time,
// neither of you should take damage
struct SINGLE_DamageQueue
{
  std::vector<std::pair<DamageEvent, float>> queue;
  float max_parry_time_s = 0.0f;
};

} // namespace game2d