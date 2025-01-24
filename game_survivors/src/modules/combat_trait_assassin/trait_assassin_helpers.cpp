#include "trait_assassin_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/system_traits/trait_helpers.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

void
handle_damage_event__trait_assassin(entt::registry& r, const DamageEvent& evt)
{
  const auto from_e = evt.from;
  const auto to_e = evt.to;

  if (!has_trait(r, evt.traits, trait_assassin_key))
    return;

  // Kill the enemy if it took damage while <20% hp.

  const int damage = calculate_damage_to_take(r, evt);
  const auto& hp_c = r.get<HealthComponent>(to_e);

  // Note: we've already taken the damage, so add the damage back
  float threshold = 0.20; // 20%
  int previous_hp = glm::min(hp_c.hp + damage, hp_c.max_hp);
  int threshold_hp = hp_c.max_hp * threshold;

  if (previous_hp <= threshold_hp && damage > 0) {
    // SDL_Log("¬¬ ASSASSINATED ¬¬");
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    dead.dead.emplace(to_e);
  }
}

} // namespace game2d