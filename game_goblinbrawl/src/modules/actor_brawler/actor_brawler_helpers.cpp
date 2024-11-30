#include "actor_brawler_helpers.hpp"
#include "actor_brawler_components.hpp"
#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_brawler/actor_brawler_components.hpp"

#include <box2d/b2_collision.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world_callbacks.h>

namespace game2d {

// should only really ever use this for small number of entities
entt::entity
brute_force_get_nearest_target(entt::registry& r, const entt::entity e, const std::vector<entt::entity>& other)
{
  const auto pos_a = get_position(r, e);

  entt::entity closest_e = entt::null;
  float closest_d = FLT_MAX;

  for (const entt::entity other_e : other) {
    if (other_e == e)
      continue;
    const auto pos_b = get_position(r, other_e);

    const auto d = pos_b - pos_a;
    const float d2 = d.x * d.x + d.y * d.y;
    if (d2 < closest_d) {
      closest_e = other_e;
      closest_d = d2;
    }
  }

  return closest_e;
};

class ClosestTargetQueryCallback : public b2QueryCallback
{
public:
  ClosestTargetQueryCallback(entt::registry& r)
    : registry(r)
  {
  }

private:
  entt::entity entity_to_exclude = entt::null;
  entt::registry& registry;

public:
  std::vector<entt::entity> targets;

  void SetEntity(entt::entity e) { entity_to_exclude = e; }

  bool ReportFixture(b2Fixture* fixture) override
  {
    const auto a = (entt::entity)fixture->GetBody()->GetUserData().pointer;

    if (a == entity_to_exclude)
      return true; // continue query, but dont process

    // filter: only search for brawler entities
    const auto* action_c = registry.try_get<ActionKey>(a);
    if (!action_c)
      return true;

    // filter: only search for other teams brawlers
    const auto team_a_idx = registry.get<TeamIndexComponent>(a).i;
    const auto team_b_idx = registry.get<TeamIndexComponent>(entity_to_exclude).i;
    if (team_a_idx == team_b_idx)
      return true;

    targets.push_back(a);
    return true; // continue query
  }
};

// use box2d to get nearest target
entt::entity
query_aabb_get_nearest_target(entt::registry& r, const entt::entity e)
{
  const auto center = get_position(r, e);
  const auto range = 1000; // large range

  b2AABB aabb;
  aabb.lowerBound = b2Vec2{ center.x, center.y } - b2Vec2(range, range);
  aabb.upperBound = b2Vec2{ center.x, center.y } + b2Vec2(range, range);

  const auto& physics_c = get_first_component<SINGLE_Physics>(r);
  ClosestTargetQueryCallback callback_func{ r };
  callback_func.targets.clear();
  callback_func.SetEntity(e);
  physics_c.world->QueryAABB(&callback_func, aabb);

  return brute_force_get_nearest_target(r, e, callback_func.targets);
};

} // namespace game2d