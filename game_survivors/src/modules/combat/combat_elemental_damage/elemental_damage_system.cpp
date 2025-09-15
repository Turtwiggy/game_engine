#include "pch.hpp"

#include "elemental_damage_system.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_elemental_damage/elemental_damage_components.hpp"
#include "modules/combat/combat_flamethrower/flamethrower_components.hpp"
#include "modules/combat/combat_weapon_type_area/combat_weapon_type_area_components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_particles/components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_combat_elemental_damage_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& evts_c = SINGLE_Events::instance;

#if defined(_DEBUG)
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto fire_state = gesert_menubar_state(menu_c, "combat_fire");
  if (fire_state.enabled)
    ImGui::Begin("DebugFire");
#endif

  for (const auto& [fixture_e, tick_c, parent_c] : r.view<TickDamageComponent, const HasParentComponent>().each()) {
    const auto par_e = parent_c.parent;

    const auto decrease_stacks = [dt](std::vector<std::pair<WEAPON_DAMAGE, float>>& vec) {
      // decrease the time
      std::for_each(vec.begin(), vec.end(), [dt](auto& p) { p.second -= dt; });

      // remove anything where time <= 0
      size_t count = std::erase_if(vec, [](const auto& p) { return p.second <= 0.0f; });
      if (count > 0)
        SDL_Log("removed %zu stacks", count);
    };
#if defined(_DEBUG)
    if (fire_state.enabled) {
      ImGui::SeparatorText(std::format("eid: {}", static_cast<uint32_t>(fixture_e)).c_str());
      for (const auto& fire_stack : tick_c.fire)
        ImGui::Text("(fire) %0.2f", fire_stack.second);
    }
#endif
    decrease_stacks(tick_c.fire);
    decrease_stacks(tick_c.ice);
    decrease_stacks(tick_c.poison);
    decrease_stacks(tick_c.shock);

    // take elemental damage every 0.5s
    tick_c.time_since_last_tick += dt;
    if (tick_c.time_since_last_tick < tick_c.time_since_last_tick_max)
      continue;
    tick_c.time_since_last_tick = 0.0f;

    const int fire_stacks = (int)tick_c.fire.size();
    const int ice_stacks = (int)tick_c.ice.size();
    const int shock_stacks = (int)tick_c.shock.size();
    const int poison_stacks = (int)tick_c.poison.size();

    // fire: while on fire, just take damage relative to the fire stacks
    if (fire_stacks > 0) {
      DamageEvent evt;
      evt.from = entt::null; // likely dead
      evt.to_parent = par_e;
      evt.to_fixture = fixture_e;
      evt.type = WEAPON_DAMAGE::KINETIC; // send kinetic so more elemental isnt applied
      evt.amount = 1.0f * fire_stacks;   // how much elemental damage?

      if (r.all_of<FlamethrowerFlameFixtureComponent>(fixture_e)) {
        const auto area_def = get_area_def(r, par_e);
        evt.amount = 1.0f * fire_stacks * area_def.stack_damage;
      }

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

#if defined(_DEBUG)
  if (fire_state.enabled)
    ImGui::End();
#endif
}

} // namespace game2d