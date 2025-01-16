
#include "event_coll_player_xp_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_coll_player_xp_components.hpp"
#include "modules/resolve_collisions/resolve_collisions_helpers.hpp"

namespace game2d {

void
handle_player_enter_xp(entt::registry& r, const OnCollisionEnter& evt)
{
  //
  // Two fixtures have collided,
  // If we choose FixtureOrBody::BODY, check the entt components on the body's user data e, not the fixture.
  // If we choose FixtureOrBody::FIXTURE, check the entt components on the fixture's user data e, not the body.
  //

  const auto [zone_e, xp_e] =
    collision_of_interest<XpZoneComponent, XpComponent>(r, evt.a, evt.b, FixtureOrBody::FIXTURE, FixtureOrBody::BODY);

  if (zone_e == entt::null || xp_e == entt::null)
    return;

  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);

  // give xp
  sxp_c.xp++;

  // TODO: play audio

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(xp_e);
}

} // namespace game2d