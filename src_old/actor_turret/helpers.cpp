#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include <algorithm>
#include <optional>

namespace game2d {

entt::entity
get_closest(entt::registry& r, const entt::entity e)
{
  const auto& physics = get_first_component<const SINGLETON_PhysicsComponent>(r);
  const auto& t = r.get<TransformComponent>(e);

  std::optional<int> idx_x;
  std::optional<int> idx_y;

  // a.x_tl < b.x_tl;
  const auto it_x = std::find(physics.sorted_x.begin(), physics.sorted_x.end(), e);
  if (it_x != physics.sorted_x.end())
    idx_x = it_x - physics.sorted_x.begin();

  // a.y_tl < b.y_tl;
  const auto it_y = std::find(physics.sorted_y.begin(), physics.sorted_y.end(), e);
  if (it_y != physics.sorted_y.end())
    idx_y = it_y - physics.sorted_y.begin();

  if (!idx_x.has_value())
    return entt::null;
  if (!idx_y.has_value())
    return entt::null;

  const auto evaluate_closest = [&r, &t](const entt::entity other, TargetInfo& info) -> bool {
    // calculate distance
    const auto& other_pos = r.get<TransformComponent>(other);
    const auto d = t.position - other_pos.position;
    const int d2 = d.x * d.x + d.y * d.y;

    if (d2 < info.distance2) {
      info.distance2 = d2;
      info.e = other;
      return true;
    }
    return false;
  };

  TargetInfo info;

  // check left...
  for (int i = idx_x.value() - 1; i >= 0; i--) {
    // check only the next in the list...
    evaluate_closest(physics.sorted_x[i], info);
    break;
  }

  // check right...
  for (int i = idx_x.value() + 1; i < physics.sorted_x.size(); i++) {
    evaluate_closest(physics.sorted_x[i], info);
    break;
  }

  // check up... (y gets less)
  for (int i = idx_y.value() - 1; i >= 0; i--) {
    evaluate_closest(physics.sorted_x[i], info);
    break;
  }

  // check down... (y gets greater)
  for (int i = idx_y.value() + 1; i < physics.sorted_y.size(); i++) {
    evaluate_closest(physics.sorted_x[i], info);
    break;
  }

  return info.e;
};

} // namespace game2d