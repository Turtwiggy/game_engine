#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/enemy/components.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"

#include <algorithm>
#include <optional>

namespace game2d {

struct ClosestInfo
{
  entt::entity e = entt::null;
  int distance2 = std::numeric_limits<int>::max();
};

void
update_turret_system(entt::registry& r, const uint64_t& ms_dt)
{
  const float dt = ms_dt / 1000.0f;
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& enemies = r.view<const EnemyComponent, const TransformComponent>();
  const auto& turrets = r.view<TurretComponent, TransformComponent>();
  for (auto [t_entity, turret, t_transform] : turrets.each()) {

    ClosestInfo info;
    std::optional<int> idx_x;
    std::optional<int> idx_y;

    // a.x_tl < b.x_tl;
    auto it_x = std::find(physics.sorted_x.begin(), physics.sorted_x.end(), t_entity);
    if (it_x != physics.sorted_x.end())
      idx_x = it_x - physics.sorted_x.begin();

    // a.y_tl < b.y_tl;
    auto it_y = std::find(physics.sorted_y.begin(), physics.sorted_y.end(), t_entity);
    if (it_x != physics.sorted_x.end())
      idx_y = it_y - physics.sorted_y.begin();

    if (!idx_x.has_value())
      continue; // this turret missing from the sorted entity list?
    if (!idx_y.has_value())
      continue; // this turret missing from the sorted entity list?

    auto evaluate_closest = [&r,
                             &t_transform](const std::vector<entt::entity>& sorted, EntityType type, int i) -> ClosestInfo {
      ClosestInfo oinfo;
      auto other_entity = sorted[i];
      auto other_type = r.get<EntityTypeComponent>(other_entity);

      // check type is of interest
      if (other_type.type != type)
        return oinfo; // early exit

      // calculate distance
      const auto& other_pos = r.get<TransformComponent>(other_entity);
      auto d = t_transform.position - other_pos.position;
      int d2 = d.x * d.x + d.y + d.y;

      // update info
      oinfo.e = other_entity;
      oinfo.distance2 = d2;
      return oinfo;
    };

    auto turret_target_type = EntityType::actor_enemy;

    // check left...
    for (int i = idx_x.value() - 1; i >= 0; i--) {
      auto oinfo = evaluate_closest(physics.sorted_x, turret_target_type, i);
      // if (oinfo.distance2 > turret.distance2_cutoff)
      //   break;
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    // check right...
    for (int i = idx_x.value() + 1; i < physics.sorted_x.size(); i++) {
      auto oinfo = evaluate_closest(physics.sorted_x, turret_target_type, i);
      // if (oinfo.distance2 > turret.distance2_cutoff)
      //   break;
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    // check up... (y gets less)
    for (int i = idx_y.value() - 1; i >= 0; i--) {
      auto oinfo = evaluate_closest(physics.sorted_x, turret_target_type, i);
      // if (oinfo.distance2 > turret.distance2_cutoff)
      //   break;
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    // check down... (y gets greater)
    for (int i = idx_y.value() + 1; i < physics.sorted_y.size(); i++) {
      auto oinfo = evaluate_closest(physics.sorted_x, turret_target_type, i);
      // if (oinfo.distance2 > turret.distance2_cutoff)
      //   break;
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    if (info.e == entt::null)
      continue;
    // the turret has a closest enemy!

    // rotate the turret to it !
    auto& e_transform = r.get<TransformComponent>(info.e);
    const auto& a = e_transform.position;
    const auto& b = t_transform.position;
    glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
    glm::vec2 nrm_dir = raw_dir;
    if (raw_dir.x != 0.0f || raw_dir.y != 0.0f)
      nrm_dir = glm::normalize(raw_dir);
    float angle = engine::dir_to_angle_radians(nrm_dir);
    // std::cout << angle << "\n";

    // rotation is not handled properly, so its only visual atm
    t_transform.rotation_radians.z = angle;

    //
    // Spawn bullet logic
    //
    turret.time_between_bullets_left -= dt;
    if (turret.time_between_bullets_left < 0.0f) {

      const float bullet_speed = 200.0f;

      // create spawn request
      CreateEntityRequest req;
      req.entity_type = turret.type_to_spawn;
      req.position = t_transform.position;
      req.velocity = glm::ivec3(-nrm_dir.x * bullet_speed, -nrm_dir.y * bullet_speed, 0);
      auto e = r.create();
      r.emplace<CreateEntityRequest>(e, req);

      // reset timer
      turret.time_between_bullets_left = turret.time_between_bullets;
    }
  }
};

} // namespace game2d