#include "event_coll_bullet_enemy_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_coll_bullet_enemy_components.hpp"
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

  auto& bullet_c = r.get<BulletComponent>(bullet_e);

  DamageEvent evt;
  evt.from = entt::null; // bullet likely dead
  evt.to = team_e;
  evt.type = DamageType::PHYSICAL;
  evt.amount = bullet_c.damage;
  evt.traits = {};
  evts_c.dispatcher->trigger(evt);
  evts_c.dispatcher->update();

  // TODO: give bullets "pierce" as the num enemies you can hit
  // auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  // dead.dead.emplace(bullet_e);
}

} // namespace game2d