#include "helpers.hpp"

#include "lifecycle/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"

namespace game2d {

entt::entity
get_gun(entt::registry& r, const entt::entity& parent)
{
  // note: identifying gun by the AttackCooldownComponent probably not good long-term
  for (const auto& [e, parent_c, cooldown] : r.view<HasParentComponent, AttackCooldownComponent>().each()) {
    if (parent_c.parent == parent)
      return e;
  }
  return entt::null;
};

} // namespace game2d