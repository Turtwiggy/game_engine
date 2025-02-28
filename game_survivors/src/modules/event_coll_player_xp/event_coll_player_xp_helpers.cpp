
#include "event_coll_player_xp_helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "event_coll_player_xp_components.hpp"
#include "modules/core_collisions/resolve_collisions_helpers.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"

namespace game2d {

void
handle_player_enter_xp(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [zone_e, xp_e] = coll<XpZoneComponent, XpComponent>(r, evt.a, evt.b);
  if (zone_e == entt::null || xp_e == entt::null)
    return;

  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);

  // give xp
  sxp_c.xp++;

  // play audio
  create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "XP_01" });

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(xp_e);
}

} // namespace game2d