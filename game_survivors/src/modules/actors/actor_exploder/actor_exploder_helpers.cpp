#include "pch.hpp"

#include "actor_exploder_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_particles/components.hpp"

namespace game2d {

class EnemyInRangeCallback : public b2QueryCallback
{
public:
  entt::registry& r;
  float nearestDistanceSquared = std::numeric_limits<float>::max();

  entt::entity self;
  std::vector<entt::entity> enemies;

  EnemyInRangeCallback(entt::registry& r_ref, entt::entity e)
    : r(r_ref)
    , self(e) {};

  bool ReportFixture(b2Fixture* fixture) override
  {
    b2Body* body = fixture->GetBody();
    const auto e = (entt::entity)body->GetUserData().pointer;

    if (e == self)
      return true; // continue query

    if (!is_enemy(body))
      return true; // continue query

    enemies.push_back(e);

    return true; // Continue the query
  }

  bool is_enemy(b2Body* body)
  {
    const entt::entity e = (entt::entity)body->GetUserData().pointer;
    if (e == entt::null || !r.valid(e))
      return false;

    const bool enemy = r.try_get<EnemyComponent>(e) != nullptr;
    return enemy;

    // bullets have TeamComponent on
    // return r.get<TeamComponent>(e).team == AvailableTeams::enemy;
  }
};

void
add_explode_on_death_callback(entt::registry& r,
                              entt::entity e,
                              const float explosion_radius_pixels,
                              const std::function<bool(entt::registry&, entt::entity)>& cond)
{
  auto& callbacks_c = r.get_or_emplace<OnDeathCallbacks>(e);

  // deal damage in area around you
  const auto explode_on_death = [cond, explosion_radius_pixels](entt::registry& r, entt::entity e) {
    GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

    // n.b.: radius so half
    const float explosion_radius_meters = pixels_to_meters(explosion_radius_pixels);
    const b2Vec2 center_m = pixels_to_meters(get_position(r, e));
    const auto things = get_all_in_area_filtered(r, center_m, explosion_radius_meters, cond);

    for (const auto& [par_e, colL_fixtures] : things) {
      if (par_e == e)
        continue; // dont damage self; you're already dead

      for (const auto& fixture_coll_result : colL_fixtures) {
        const auto fixture_e = fixture_coll_result.fixture_e;
        const bool has_hp = r.try_get<HealthComponent>(fixture_e);
        if (!has_hp)
          continue; // shield or xp zone or something without health

        const bool is_player = r.try_get<PlayerComponent>(par_e);
        const auto& tag_c = r.get<TagComponent>(par_e);
        SDL_Log("Exploooosion! hit: %s", tag_c.tag.c_str());

        // Send explosion damage event
        DamageEvent evt;
        evt.from = entt::null; // likely dead
        evt.to = fixture_e;
        evt.type = DamageType::PHYSICAL;
        evt.amount = is_player ? 4 : 100; // todo: replace with "correct" damage for explosion
        evts_c.dispatcher->trigger(evt);
        evts_c.dispatcher->update();
        break; // if you collide with a valid fixture, damage once
      }
    }
  };
  callbacks_c.callbacks.push_back(explode_on_death);

  // Big explosion when ded
  //
  const auto spawn_particles_callback = [explosion_radius_pixels](entt::registry& r, entt::entity e) {
    RequestToSpawnParticles request;
    request.key = "death_exploder";
    request.position = get_position(r, e);
    request.radius = explosion_radius_pixels;
    create_empty<RequestToSpawnParticles>(r, request);
  };
  callbacks_c.callbacks.push_back(spawn_particles_callback);
}

} // namespace game2d