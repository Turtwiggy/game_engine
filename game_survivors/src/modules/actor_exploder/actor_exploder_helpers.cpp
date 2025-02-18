#include "actor_exploder_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_damage/event_damage_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_particles/components.hpp"

#include <box2d/b2_fixture.h>
#include <box2d/b2_world_callbacks.h>

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
add_explode_on_death_callback(entt::registry& r, entt::entity e)
{
  auto& callbacks_c = r.get<OnDeathCallbacks>(e);

  // deal damage in area around you
  const auto explode_on_death = [](entt::registry& r, entt::entity e) {
    GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

    // n.b.: radius so half
    const float explosion_radius_meters = pixels_to_meters(explosion_radius_pixels);

    const b2Vec2 center_m = pixels_to_meters(get_position(r, e));
    const std::function<bool(entt::registry&, entt::entity)> is_enemy = [](entt::registry& r, entt::entity e) -> bool {
      return r.try_get<EnemyComponent>(e) != nullptr;
    };
    auto enemies = get_all_in_area_filtered(r, center_m, explosion_radius_meters, is_enemy);

    for (const auto& [d2, other_e] : enemies) {
      // const auto& tag_c = r.get<TagComponent>(other_e);
      // const auto& tag = tag_c.tag;

      // Note: specifying the fixture to damage here seems wrong
      auto core_e = get_fixture_by_tag(r, other_e, "fixture_core");

      DamageEvent evt;
      evt.from = entt::null; // likely dead
      evt.to = core_e;
      evt.type = DamageType::PHYSICAL;
      evt.amount = 100; // todo: replace with "correct" damage for explosion
      evts_c.dispatcher->trigger(evt);
      evts_c.dispatcher->update();

      // Send explosion damage event
    }
  };
  callbacks_c.callbacks.push_back(explode_on_death);

  // Big explosion when ded
  //
  const auto spawn_particles_callback = [](entt::registry& r, entt::entity e) {
    RequestToSpawnParticles request;
    request.key = "death_exploder";
    request.position = get_position(r, e);
    create_empty<RequestToSpawnParticles>(r, request);
  };
  callbacks_c.callbacks.push_back(spawn_particles_callback);
}

} // namespace game2d