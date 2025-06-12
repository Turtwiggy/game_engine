#include "pch.hpp"

#include "elemental_damage_system.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
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
  auto& evts_c = SINGLE_Events::instance;

  for (const auto& [fixture_e, tick_c, parent_c] : r.view<TickDamageComponent, const HasParentComponent>().each()) {
    const auto par_e = parent_c.parent;

    if (par_e == entt::null || !r.valid(par_e))
      continue;

    // take elemental damage every 0.5s
    tick_c.time_since_last_damage += dt;
    if (tick_c.time_since_last_damage < tick_c.time_since_last_damage_max)
      continue;
    tick_c.time_since_last_damage = 0.0f;

    const auto decrease_stacks = [dt](auto& vec) {
      // When the time is less than 0, remove the stack.
      for (int i = 0; auto& [type, time] : vec) {
        time -= dt;
        if (time <= 0.0f)
          vec.erase(vec.begin() + i);
        else
          i++;
      }
    };
    decrease_stacks(tick_c.fire);
    decrease_stacks(tick_c.ice);
    decrease_stacks(tick_c.poison);
    decrease_stacks(tick_c.shock);

    const int fire_stacks = (int)tick_c.fire.size();
    const int ice_stacks = (int)tick_c.ice.size();
    const int shock_stacks = (int)tick_c.shock.size();
    const int poison_stacks = (int)tick_c.poison.size();

    // fire: while on fire, just take damage relative to the fire stacks
    if (fire_stacks > 0) {
      DamageEvent evt;
      evt.from = entt::null;             // likely dead
      evt.to = fixture_e;                // fixture_e
      evt.type = WEAPON_DAMAGE::KINETIC; // send kinetic so more elemental isnt applied
      evt.amount = 1.0f * fire_stacks;   // how much elemental damage?
      evts_c.dispatcher->trigger(evt);

      // Request some fire particles at this pos while on fire
      RequestToSpawnParticles request;
      request.key = "fire_particles";
      request.position = get_position(r, par_e);
      create_empty<RequestToSpawnParticles>(r, request);
    }

    // ice affects speed
    // note: if any other system affects current_speed,
    // need to change the variable for something better
    {
      auto& speed_c = r.get<ActorSpeedComponent>(par_e);

      // make it so that 10 ice stacks completely stop something
      const float slow_amount = (speed_c.base_speed / (float)10) * ice_stacks;

      speed_c.current_speed = glm::max(speed_c.base_speed - slow_amount, 0.0f);
      if (speed_c.current_speed == 0.0f)
        SDL_Log("maybe freeze the enemy?");

      if (ice_stacks > 0) {
        RequestToSpawnParticles request;
        request.key = "ice_particles";
        request.position = get_position(r, par_e);
        create_empty<RequestToSpawnParticles>(r, request);
      }
    }

    // todo: shock, poison

    //
  }

  evts_c.dispatcher->update();
}

} // namespace game2d