#include "pch.hpp"

#include "event_coll_player_xp_helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "event_coll_player_xp_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/collisions/resolve_collisions_helpers.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"

namespace game2d {

void
handle_player_enter_xp(entt::registry& r, const OnCollisionEnter& evt)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  GET_FIRST_OR_RETURN(SINGLE_PostFixedUpdateCallbacks, r, callbacks_e, callbacks_c);

  // If your xp zone collides with xp, make it fly to the player.
  {
    const auto [zone_e, xp_fixture_e] = coll<XpZoneComponent, XpComponent>(r, evt.a, evt.b);
    if (zone_e != entt::null && xp_fixture_e != entt::null) {

      const std::function<void(entt::registry&)> make_xp_fly_to_player = [xp_fixture_e, zone_e](entt::registry& r) {
        const auto& fixture_c = r.get<PhysicsFixtureComponent>(xp_fixture_e);
        const auto player_par_e = r.get<HasParentComponent>(zone_e).parent;

        // change xp from static to dynamic.
        b2Body_SetType(fixture_c.bodyId, b2BodyType::b2_dynamicBody);

        // Add components to xp parent not fixture.
        const auto xp_parent = r.get<HasParentComponent>(xp_fixture_e).parent;
        ApplyForceToDynamicTarget tgt_c;
        tgt_c.orbit = false;
        tgt_c.reduce_thrusters = false;
        r.emplace_or_replace<ApplyForceToDynamicTarget>(xp_parent, tgt_c);
        r.emplace_or_replace<PhysicsDynamicTarget>(xp_parent, player_par_e);
      };

      callbacks_c.callbacks.push_back(make_xp_fly_to_player);
    }
  }

  // if the player body collides with x, give the xp.
  {
    const auto [player_fix_e, xp_e] = coll<PlayerFixtureComponent, XpComponent>(r, evt.a, evt.b);
    if (player_fix_e != entt::null && xp_e != entt::null) {

      // give xp
      const auto& xp_c = r.get<XpComponent>(xp_e);
      const bool give_levelup = xp_c.levelup;
      if (give_levelup)
        sxp_c.xp += sxp_c.xp_for_next_level - sxp_c.xp; // give the rest of the level
      else
        sxp_c.xp++;

      auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
      dead.dead.push_back(xp_e);

      // play audio
      create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "XP_0" });
    }
  }
}

} // namespace game2d