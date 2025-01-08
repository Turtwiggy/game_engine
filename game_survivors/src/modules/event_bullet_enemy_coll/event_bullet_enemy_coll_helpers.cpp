#include "event_bullet_enemy_coll_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_Damage/event_damage_components.hpp"
#include "modules/system_autofire/autofire_components.hpp"

namespace game2d {

void
handle_bullet_enemy_coll(entt::registry& r, const OnCollisionEnter& coll_evt)
{
  const auto [bullet_e, team_e] = collision_of_interest<BulletComponent, TeamComponent>(r, coll_evt.a, coll_evt.b);
  if (bullet_e == entt::null || team_e == entt::null)
    return;
  if (r.get<TeamComponent>(bullet_e).team != AvailableTeams::player)
    return;
  if (r.get<TeamComponent>(team_e).team != AvailableTeams::enemy)
    return;
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  DamageEvent evt;
  evt.from = bullet_e;
  evt.to = team_e;
  evt.type = DamageType::PHYSICAL;
  evt.amount = 10;
  evt.traits = {};
  evts_c.dispatcher->trigger(evt);
  evts_c.dispatcher->update();
}

} // namespace game2d