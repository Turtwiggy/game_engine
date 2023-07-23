#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/enemy/components.hpp"
#include "modules/player/components.hpp"
#include "modules/turret/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_turret_system(entt::registry& r, const uint64_t& ms_dt)
{
  const float dt = ms_dt / 1000.0f;
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& enemies = r.view<const EnemyComponent, const TransformComponent>();
  const auto& turrets = r.view<TurretComponent, TransformComponent>();
  const auto& player = get_first<PlayerComponent>(r); // assume one player for now

  for (auto [t_entity, turret, t_transform] : turrets.each()) {

    const auto info = get_closest(r, t_entity, t_transform, physics, EntityType::actor_enemy);
    if (info.e == entt::null)
      continue; // no enemy

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

    // Spawn bullet logic

    if (turret.active)
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