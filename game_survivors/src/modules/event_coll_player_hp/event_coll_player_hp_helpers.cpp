#include "event_coll_player_hp_helpers.hpp"

#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "event_coll_player_hp_components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"

namespace game2d {

void
handle_player_enter_hp(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [zone_e, item_e] = coll<XpZoneComponent, ItemHealingPackComponent>(r, evt.a, evt.b);
  if (zone_e == entt::null || item_e == entt::null)
    return;
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  // WantToHealEvent heal_evt;
  // evts_c.dispatcher->trigger(heal_evt);
  // evts_c.dispatcher->update();

  SDL_Log("collided with hp.. healing...");

  const auto par_e = r.get<HasParentComponent>(zone_e).parent;
  const auto fix_e = get_fixture_by_tag(r, par_e, "fixture_player");
  auto& hp_c = r.get<HealthComponent>(fix_e);
  const auto amount = hp_c.max_hp * 1.0; // heal 100%
  hp_c.hp += amount;
  hp_c.hp = glm::min(hp_c.hp, hp_c.max_hp);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(item_e);
}

} // namespace game2d