#pragma once

#include "modules/actors/actor_enemy/components.hpp"
#include <entt/fwd.hpp>

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

  bool ReportFixture(b2Fixture* fixture) override;
  bool is_enemy(b2Body* body);
};

void
add_explode_on_death_callback(entt::registry& r,
                              entt::entity e,
                              const float explosion_radius_pixels,
                              const std::function<bool(entt::registry&, entt::entity)>& cond,
                              const std::string key);

} // namespace game2d