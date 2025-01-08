#include "autofire_system.hpp"

#include "autofire_components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include <box2d/b2_collision.h>

namespace game2d {

class NearestEnemyCallback : public b2QueryCallback
{
public:
  entt::registry& r;
  float nearestDistanceSquared = std::numeric_limits<float>::max();
  b2Vec2 position;

  b2Body* nearestEnemy = nullptr;
  entt::entity nearest_e = entt::null;

  NearestEnemyCallback(entt::registry& r_ref, const b2Vec2& pos)
    : r(r_ref)
    , position(pos)
  {
  }

  bool ReportFixture(b2Fixture* fixture) override
  {
    b2Body* body = fixture->GetBody();

    if (!is_enemy(body))
      return true;

    // Calculate the distance squared (avoiding sqrt for performance)
    b2Vec2 diff = body->GetPosition() - position;
    float distanceSquared = diff.LengthSquared();

    if (distanceSquared < nearestDistanceSquared) {
      nearestDistanceSquared = distanceSquared;
      nearestEnemy = body;
      nearest_e = (entt::entity)body->GetUserData().pointer;
    }

    return true; // Continue the query
  }

  // Example placeholder for identifying enemies
  bool is_enemy(b2Body* body)
  {
    const entt::entity e = (entt::entity)body->GetUserData().pointer;
    return r.get<TeamComponent>(e).team == AvailableTeams::enemy;
  }
};

void
update_autofire_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_Physics, r, phys_e, phys_c);

  const float search_radius = 500.0f; // for nearest enemy

  for (const auto& [wep_e, t_c, wep_c, cooldown_c] :
       r.view<const TransformComponent, const WeaponComponent, CooldownComponent>().each()) {
    //
    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    // get closest enemy
    //
    NearestEnemyCallback callback(r, { t_c.position.x, t_c.position.y });
    b2AABB aabb;
    aabb.lowerBound = b2Vec2{ t_c.position.x, t_c.position.y } - b2Vec2{ search_radius, search_radius };
    aabb.upperBound = b2Vec2{ t_c.position.x, t_c.position.y } + b2Vec2{ search_radius, search_radius };
    phys_c.world->QueryAABB(&callback, aabb);

    auto nearest_e = callback.nearest_e;
    if (nearest_e == entt::null)
      continue;

    const auto dir = get_position(r, nearest_e) - get_position(r, wep_e);

    auto bullet_e = spawn(r, "bullet_default");
    give_life(r, bullet_e, get_position(r, wep_e), { 6, 6 });
    r.emplace<BulletComponent>(bullet_e);
    r.emplace<TeamComponent>(bullet_e, AvailableTeams::player);
    r.get<PhysicsBodyComponent>(bullet_e).base_speed = 100.0f;
    r.emplace<EntityTimedLifecycle>(bullet_e, 3 * 1000);

    // set velocity
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    body_c.body->SetLinearVelocity({ body_c.base_speed * dir.x, body_c.base_speed * dir.y });
  }
}

} // namespace game2d