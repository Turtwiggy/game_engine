#include "system.hpp"

#include "components.hpp"

#include "maths/maths.hpp"
#include "modules//enemy/components.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_turret_system(entt::registry& r, const uint64_t& ms_dt)
{
  const auto& enemies = r.view<const EnemyComponent, const TransformComponent>();
  const auto& turrets = r.view<const TurretComponent, const TransformComponent>();

  for (auto [t_entity, turret, t_transform] : turrets.each()) {

    // I have an AABB physics sweep-and-prune implementation, could use that
    // Could use box2d

    float closest_distance = std::numeric_limits<float>::max();
    entt::entity closest_enemy = entt::null;

    // This code is too slow :'(
    // for (auto [e_entity, enemy, e_transform] : enemies.each()) {
    //   auto d = t_transform.position - e_transform.position;
    //   auto d2 = d.x * d.x + d.y + d.y;
    //   if (d2 < closest_distance) {
    //     closest_distance = d2;
    //     closest_enemy = e_entity;
    //   }
    // }
    // if (closest_enemy == entt::null)
    //   continue;
    // the turret has a closest enemy!

    // rotate the turret to it!
    // auto& e_transform = r.get<TransformComponent>(closest_enemy);
    // const auto& a = e_transform.position;
    // const auto& b = t_transform.position;
    // glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
    // glm::vec2 nrm_dir = raw_dir;
    // if (raw_dir.x != 0.0f && raw_dir.y != 0.0f)
    //   nrm_dir = glm::normalize(raw_dir);
    // float angle = engine::dir_to_angle_radians(nrm_dir);
    // std::cout << angle << "\n";

    // t_transform.rotation_radians.z = angle;
  }
};

} // namespace game2d