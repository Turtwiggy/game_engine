#include "autofire_helpers.hpp"

#include "autofire_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/combat/components.hpp"

namespace game2d {

void
handle_bullet_enemy_coll(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [bullet_e, team_e] = collision_of_interest<BulletComponent, TeamComponent>(r, evt.a, evt.b);
  if (bullet_e == entt::null || team_e == entt::null)
    return;
  if (r.get<TeamComponent>(bullet_e).team != AvailableTeams::player)
    return;
  if (r.get<TeamComponent>(team_e).team != AvailableTeams::enemy)
    return;

  // for the moment, just kill things, rather than dealing damage
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(bullet_e);
  dead.dead.emplace(team_e);
}

} // namespace game2d