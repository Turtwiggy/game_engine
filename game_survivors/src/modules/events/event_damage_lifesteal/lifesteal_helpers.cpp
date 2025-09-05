#include "pch.hpp"

#include "lifesteal_helpers.hpp"

#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "lifesteal_components.hpp"
#include "modules/combat/combat_core/components.hpp"

namespace game2d {

void
handle_damage_event_lifesteal(entt::registry& r, const DamageEvent& evt)
{
  if (evt.from == entt::null)
    return; // lifesteal on bullet

  //
  // note: because this is in a separete function,
  // we dont monitor if the defending entity dodged.
  // it might make sense not to lifesteal if the entity dodged.
  //

  auto* bullet_lifesteal_c = r.try_get<BulletLifesteal>(evt.from);
  if (!bullet_lifesteal_c)
    return;

  const auto wep_e = r.get<HasParentComponent>(evt.from).parent;
  const auto par_e = r.get<HasParentComponent>(wep_e).parent;
  const auto par_fixture_e = get_fixture_by_tag(r, par_e, "fixture_player");

  // do the lifesteal
  const auto lifesteal_mul = bullet_lifesteal_c->percent_0_100 / 100.0f;
  auto& hp_c = r.get<HealthComponent>(par_fixture_e);
  const auto lifesteal_amount = hp_c.max_hp * lifesteal_mul;
  hp_c.hp += lifesteal_amount;
  hp_c.hp = glm::min(hp_c.hp, hp_c.max_hp);
}

} // namespace game2d