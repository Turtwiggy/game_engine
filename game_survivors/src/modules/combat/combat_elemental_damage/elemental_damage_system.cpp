#include "pch.hpp"

#include "elemental_damage_system.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_elemental_damage/elemental_damage_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_particles/components.hpp"

namespace game2d {

void
update_combat_elemental_damage_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  for (const auto& [e, tick_c, parent_c] : r.view<TickDamageComponent, HasParentComponent>().each()) {
    const auto par_e = parent_c.parent;

    // Fire stacks decrease over time.
    // When the time is less than 0, remove the stack.
    for (int i = 0; auto& [type, time] : tick_c.fire) {
      time -= dt;
      if (time <= 0.0f)
        tick_c.fire.erase(tick_c.fire.begin() + i);
      else
        i++;
    }

    const int fire_stacks = (int)tick_c.fire.size();
    const int ice_stacks = (int)tick_c.ice.size();
    const int shock_stacks = (int)tick_c.shock.size();
    const int poison_stacks = (int)tick_c.poison.size();

    // take elemental damage every 0.5s
    tick_c.time_since_last_damage += dt;
    if (tick_c.time_since_last_damage >= tick_c.time_since_last_damage_max) {

      // fire: while on fire, just take damage relative to the fire stacks
      if (fire_stacks > 0) {
        DamageEvent evt;
        evt.from = entt::null;             // likely dead
        evt.to = e;                        // fixture_e
        evt.type = WEAPON_DAMAGE::KINETIC; // send kinetic so more elemental isnt applied
        evt.amount = 1.0f * fire_stacks;   // how much elemental damage?
        evts_c.dispatcher->trigger(evt);

        // Request some fire particles at this pos while on fire
        RequestToSpawnParticles request;
        request.key = "fire_particles";
        request.position = get_position(r, par_e);
        create_empty<RequestToSpawnParticles>(r, request);
      }

      // todo: ice, shock, poison

      tick_c.time_since_last_damage = 0.0f;
    }

    //
  }

  evts_c.dispatcher->update();
}

} // namespace game2d