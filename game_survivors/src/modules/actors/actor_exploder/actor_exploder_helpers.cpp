#include "pch.hpp"

#include "actor_exploder_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_particles/components.hpp"

namespace game2d {

void
add_explode_on_death_callback(entt::registry& r,
                              entt::entity e,
                              const float explosion_radius_pixels,
                              const std::function<bool(entt::registry&, entt::entity)>& cond,
                              const ParticleType particle_type)
{
  auto& callbacks_c = r.get_or_emplace<OnDeathCallbacks>(e);

  // deal damage in area around you
  const auto explode_on_death = [cond, explosion_radius_pixels](entt::registry& r, entt::entity e) {
    auto& evts_c = SINGLE_Events::instance;

    // n.b.: radius so half
    const float explosion_radius_meters = pixels_to_meters(explosion_radius_pixels);
    const b2Vec2 center_m = pixels_to_meters(get_position(r, e));
    const auto things = get_all_in_area_filtered(r, center_m, explosion_radius_meters, cond);

    for (const auto& [par_e, colL_fixtures] : things) {
      if (par_e == e)
        continue; // dont damage self; you're already dead

      for (const auto& fixture_coll_result : colL_fixtures) {
        const auto fixture_e = fixture_coll_result.fixture_e;
        const bool has_hp = r.all_of<HealthComponent>(fixture_e);
        if (!has_hp)
          continue; // shield or xp zone or something without health

        const bool is_player = r.all_of<PlayerBoatComponent>(par_e);
        const auto& tag_c = r.get<TagComponent>(par_e);
        SDL_Log("Exploooosion! hit: %s", tag_c.tag.c_str());

        float explosion_damage = 50.0f;
        if (is_player) {
          // take off 30% of the players max health.
          explosion_damage = r.get<const HealthComponent>(fixture_e).max_hp * 0.30f;
        }

        // Send explosion damage event
        const DamageEvent evt{
          .from = entt::null, // exploder probably just died
          .to_parent = par_e,
          .to_fixture = fixture_e,
          .amount = explosion_damage,
          .type = WEAPON_DAMAGE::KINETIC,
        };
        evts_c.dispatcher->trigger(evt);
        evts_c.dispatcher->update();
        break; // if you collide with a valid fixture, damage once
      }
    }
  };
  callbacks_c.callbacks.push_back(explode_on_death);

  // Big explosion when ded
  //
  const auto spawn_particles_callback = [particle_type, explosion_radius_pixels](entt::registry& r, entt::entity e) {
    RequestToSpawnParticles request;
    request.particle_type = particle_type;
    request.position = get_position(r, e);
    request.radius_pixels_upper = explosion_radius_pixels;
    create_empty<RequestToSpawnParticles>(r, request);
  };
  callbacks_c.callbacks.push_back(spawn_particles_callback);
}

} // namespace game2d