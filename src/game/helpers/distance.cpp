#include "distance.hpp"

#include "game/modules/combat/components.hpp"

namespace game2d {

entt::entity
get_nearest_attackable(Game& g, const entt::entity& e)
{
  auto& r = g.state;

  float distance_squared = std::numeric_limits<float>::max();
  entt::entity nearest = entt::null;

  const TransformComponent& player_transform = r.get<TransformComponent>(e);
  const auto& view = r.view<TransformComponent, const TakeDamageComponent>();
  for (auto [entity, transform, damageable] : view.each()) {

    if (entity == e)
      continue;

    auto& a = player_transform.position;
    auto& b = transform.position;
    const float calc = pow(a.x - b.x, 2) + pow(a.y - b.y, 2);

    if (calc < distance_squared)
      nearest = entity;
  };

  return nearest;
};

}