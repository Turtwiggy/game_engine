#include "actor_exploder_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/components.hpp"
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

  std::vector<entt::entity> enemies;

  EnemyInRangeCallback(entt::registry& r_ref)
    : r(r_ref) {};

  bool ReportFixture(b2Fixture* fixture) override
  {
    b2Body* body = fixture->GetBody();

    if (!is_enemy(body))
      return true;

    auto e = (entt::entity)body->GetUserData().pointer;
    enemies.push_back(e);

    return true; // Continue the query
  }

  // Example placeholder for identifying enemies
  bool is_enemy(b2Body* body)
  {
    const entt::entity e = (entt::entity)body->GetUserData().pointer;
    bool is_null = e == entt::null;
    bool is_valid = r.valid(e);
    if (is_null || !is_valid)
      return false;
    auto& tag_c = r.get<TagComponent>(e);
    return r.get<TeamComponent>(e).team == AvailableTeams::enemy;
  }
};

std::vector<entt::entity>
enemies_in_range(entt::registry& r, entt::entity e, float radius)
{
  const auto phys_e = get_first<SINGLE_Physics>(r);
  if (phys_e == entt ::null)
    return {};
  auto& phys_c = r.get<SINGLE_Physics>(phys_e);

  const auto pos = get_position(r, e);

  EnemyInRangeCallback callback(r);
  b2AABB aabb;
  aabb.lowerBound = b2Vec2{ pos.x, pos.y } - b2Vec2{ radius, radius };
  aabb.upperBound = b2Vec2{ pos.x, pos.y } + b2Vec2{ radius, radius };
  phys_c.world->QueryAABB(&callback, aabb);

  return callback.enemies;
};

void
add_explode_on_death_callback(entt::registry& r, entt::entity e)
{
  auto& callbacks_c = r.get<OnDeathCallbacks>(e);

  // TODO: deal damage in area around you
  auto explode_on_death = [](entt::registry& r, entt::entity e) {
    GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

    // n.b.: radius so half
    auto enemies = enemies_in_range(r, e, explosion_radius);

    for (const auto& other_e : enemies) {

      DamageEvent evt;
      evt.from = entt::null; // likely dead
      evt.to = other_e;
      evt.type = DamageType::PHYSICAL;
      evt.amount = 100; // todo: replace with "correct" damage for explosion
      evt.traits = {};
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