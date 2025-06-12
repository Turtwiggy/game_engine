#include "pch.hpp"

#include "event_coll_player_hp_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_coll_player_hp_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

void
handle_player_enter_hp(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [pfixture_e, item_e] = coll<PlayerFixtureComponent, ItemHealingPackComponent>(r, evt.a, evt.b);
  if (pfixture_e == entt::null || item_e == entt::null)
    return;
  auto& evts_c = SINGLE_Events::instance;

  // WantToHealEvent heal_evt;
  // evts_c.dispatcher->trigger(heal_evt);
  // evts_c.dispatcher->update();

  SDL_Log("collided with hp.. healing...");

  auto& hp_c = r.get<HealthComponent>(pfixture_e);
  const auto amount = hp_c.max_hp * 1.0; // heal 100%
  hp_c.hp += amount;
  hp_c.hp = glm::min(hp_c.hp, hp_c.max_hp);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.push_back(item_e);
}

} // namespace game2d
