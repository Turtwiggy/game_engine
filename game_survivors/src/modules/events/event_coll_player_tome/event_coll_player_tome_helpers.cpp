#include "pch.hpp"

#include "event_coll_player_tome_helpers.hpp"

#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_tome/tome_components.hpp"

namespace game2d {

void
handle_player_enter_tome(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [pfixture_e, item_e] = coll<PlayerFixtureComponent, TomeComponent>(r, evt.a, evt.b);
  if (pfixture_e == entt::null || item_e == entt::null)
    return;

  SDL_Log("You collided with a tome!");

  // WeaponLevelReachedEvent lv_evt;
  // lv_evt.level = wep_level_c.level;
  // lv_evt.par_e = par_e;
  // lv_evt.wep_e = weapon_e;
  // evts_c.dispatcher->trigger(lv_evt);
  // evts_c.dispatcher->update();
}

} // namespace game2d