
#include "event_coll_player_xp_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_coll_player_xp_components.hpp"
#include "modules/actor_player/components.hpp"

namespace game2d {

void
handle_player_enter_xp(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [player_e, xp_e] = collision_of_interest<PlayerComponent, XpComponent>(r, evt.a, evt.b);
  if (player_e == entt::null || xp_e == entt::null)
    return;
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);

  // give xp
  sxp_c.xp++;

  // TODO: play audio

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(xp_e);
}

} // namespace game2d