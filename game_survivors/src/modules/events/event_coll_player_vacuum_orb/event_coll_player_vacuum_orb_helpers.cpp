#include "pch.hpp"

#include "event_coll_player_vacuum_orb_helpers.hpp"

#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "event_coll_player_vacuum_orb_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"

namespace game2d {

void
handle_player_enter_vacuum_orb(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [pfixture_e, item_e] = coll<PlayerFixtureComponent, ItemVacuumOrbComponent>(r, evt.a, evt.b);
  if (pfixture_e == entt::null || item_e == entt::null)
    return;

  SDL_Log("You collided with a vacuum orb");

  // Remove all xp items from the physics system.
  // Make them all fly to the player.
  // Give that amount of xp if the distance is < than some amount to the player

  auto vacuum_orbs_on_death = [pfixture_e](entt::registry& r, const entt::entity dead_e) {
    const auto player_par_e = r.get<HasParentComponent>(pfixture_e).parent;

    const auto xp_view = r.view<XpComponent, PhysicsFixtureComponent>();
    for (const auto& [xp_e, xp_c, fixture_c] : xp_view.each()) {

      // change xp from static to dynamic.
      fixture_c.body->SetType(b2BodyType::b2_dynamicBody);

      // Add components to xp parent not fixture.
      const auto xp_parent = r.get<HasParentComponent>(xp_e).parent;
      ApplyForceToDynamicTarget tgt_c;
      tgt_c.orbit = false;
      tgt_c.speed = 10.0f;
      tgt_c.reduce_thrusters = false;
      r.emplace_or_replace<ApplyForceToDynamicTarget>(xp_parent, tgt_c);
      r.emplace_or_replace<PhysicsDynamicTarget>(xp_parent, player_par_e);
    }
  };

  //
  // use callbacks when need to update physics things,
  // because currently collision via physics world
  // and updating things during physics step makes physics sad
  //

  auto& callbacks_c = r.get_or_emplace<OnDeathCallbacks>(item_e);
  callbacks_c.callbacks.push_back(vacuum_orbs_on_death);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.push_back(item_e);
}

} // namespace game2d