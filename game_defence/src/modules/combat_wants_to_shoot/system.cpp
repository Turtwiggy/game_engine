#include "system.hpp"

#include "components.hpp"
#include "modules/combat_damage/components.hpp"

namespace game2d {

void
update_wants_to_shoot_system(entt::registry& r)
{
  // continuously shoot if a target is set
  //
  const auto& auto_shoot_view = r.view<DynamicTargetComponent>();

  for (const auto& [e, target] : auto_shoot_view.each()) {
    const auto& t = target.target;

    if (!r.valid(t) || t == entt::null) {
      r.remove<DynamicTargetComponent>(e);
      continue;
    }

    const auto& hp = r.get<HealthComponent>(t);
    if (hp.hp <= 0) {
      r.remove<DynamicTargetComponent>(e);
      continue;
    }

    // otherwise, shoot it
    r.emplace_or_replace<WantsToShoot>(e);
  }
}

} // namespace game2d