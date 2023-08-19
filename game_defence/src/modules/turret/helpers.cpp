#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "modules/physics/components.hpp"
#include "modules/spawner/components.hpp" // hack: shouldnt be here
#include "renderer/components.hpp"

#include <algorithm>
#include <optional>

namespace game2d {

ClosestInfo
get_closest(entt::registry& r, const entt::entity& e, const std::vector<EntityType>& types)
{
  const auto& physics = get_first_component<const SINGLETON_PhysicsComponent>(r);
  const auto& t = r.get<TransformComponent>(e);

  ClosestInfo info;

  std::optional<int> idx_x;
  std::optional<int> idx_y;

  // a.x_tl < b.x_tl;
  auto it_x = std::find(physics.sorted_x.begin(), physics.sorted_x.end(), e);
  if (it_x != physics.sorted_x.end())
    idx_x = it_x - physics.sorted_x.begin();

  // a.y_tl < b.y_tl;
  auto it_y = std::find(physics.sorted_y.begin(), physics.sorted_y.end(), e);
  if (it_y != physics.sorted_y.end())
    idx_y = it_y - physics.sorted_y.begin();

  if (!idx_x.has_value())
    return info; // this turret missing from the sorted entity list?
  if (!idx_y.has_value())
    return info; // this turret missing from the sorted entity list?

  auto evaluate_closest = [&r, &t, &types](const std::vector<entt::entity>& sorted, int i) -> ClosestInfo {
    ClosestInfo oinfo;
    auto other_entity = sorted[i];
    auto other_type = r.get<EntityTypeComponent>(other_entity);

    const bool type_of_interest = std::find(types.begin(), types.end(), other_type.type) != types.end();
    if (!type_of_interest)
      return oinfo; // early exit

    // HACK....
    // make sure to only target enemy spawners
    if (other_type.type == EntityType::spawner) {
      const auto& spawning = r.get<SpawnerComponent>(other_entity);
      if (spawning.type_to_spawn != EntityType::actor_enemy)
        return oinfo; // early exit
    }

    // calculate distance
    const auto& other_pos = r.get<TransformComponent>(other_entity);
    auto d = t.position - other_pos.position;
    int d2 = d.x * d.x + d.y * d.y;

    // update info
    oinfo.e = other_entity;
    oinfo.distance2 = d2;
    return oinfo;
  };

  // check left...
  for (int i = idx_x.value() - 1; i >= 0; i--) {
    const auto oinfo = evaluate_closest(physics.sorted_x, i);
    if (oinfo.distance2 < info.distance2)
      info = oinfo;
  }

  // check right...
  for (int i = idx_x.value() + 1; i < physics.sorted_x.size(); i++) {
    const auto oinfo = evaluate_closest(physics.sorted_x, i);
    if (oinfo.distance2 < info.distance2)
      info = oinfo;
  }

  // check up... (y gets less)
  for (int i = idx_y.value() - 1; i >= 0; i--) {
    const auto oinfo = evaluate_closest(physics.sorted_y, i);
    if (oinfo.distance2 < info.distance2)
      info = oinfo;
  }

  // check down... (y gets greater)
  for (int i = idx_y.value() + 1; i < physics.sorted_y.size(); i++) {
    const auto oinfo = evaluate_closest(physics.sorted_y, i);
    if (oinfo.distance2 < info.distance2)
      info = oinfo;
  }

  return info;
};

} // namespace game2d