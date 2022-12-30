#include "distance.hpp"

#include "modules/combat/components.hpp"

namespace game2d {

entt::entity
get_nearest_attackable(Game& g, const entt::entity& e)
{
  const auto& r = g.state;

  float distance_squared = std::numeric_limits<float>::max();
  entt::entity nearest = entt::null;

  const TransformComponent& player_transform = r.get<TransformComponent>(e);

  const auto& view = r.view<TransformComponent, const TakeDamageComponent>();
  for (auto [entity, transform, damageable] : view.each()) {

    if (entity == e)
      continue;

    const auto& a = player_transform.position;
    const auto& b = transform.position;
    const float calc = pow(b.x - a.x, 2) + pow(b.y - a.y, 2);

    if (calc < distance_squared) {
      nearest = entity;
      distance_squared = calc;
    }
  };

  return nearest;
};

} // namespace game2d