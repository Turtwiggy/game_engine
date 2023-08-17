#include "system.hpp"

#include "components.hpp"
#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/enemy/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

namespace game2d {

void
update_turret_system(entt::registry& r, const uint64_t& ms_dt)
{
  const float dt = ms_dt / 1000.0f;
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& enemies = r.view<const EnemyComponent, const TransformComponent>();
  const auto& turrets = r.view<TurretComponent, TransformComponent>();
  const auto& player = get_first<PlayerComponent>(r); // assume one player for now

  for (auto [t_entity, turret, t] : turrets.each()) {

    //
    // Get Closest Entity to turret
    //
    const auto info = get_closest(r, t_entity, EntityType::actor_enemy);

    if (info.e == entt::null)
      continue; // no enemy

    // get the dir to the target
    auto& e_transform = r.get<TransformComponent>(info.e);
    const auto& a = e_transform.position;
    const auto& b = t.position;
    glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
    glm::vec2 nrm_dir = raw_dir;
    if (raw_dir.x != 0.0f || raw_dir.y != 0.0f)
      nrm_dir = glm::normalize(raw_dir);

    // offset the bullet by a distance
    // to stop the bullet spawning inside the turret
    glm::ivec2 offset_pos = { t.position.x - nrm_dir.x * 20.0f, t.position.y - nrm_dir.y * 20.0f };

    // rotation is not handled, so its only visual atm
    // float angle = engine::dir_to_angle_radians(nrm_dir);
    // t.rotation_radians.z = angle;

    //
    // request to shoot
    //

    if (turret.active)
      turret.time_between_bullets_left -= dt;

    if (turret.time_between_bullets_left < 0.0f) {

      const float bullet_speed = 200.0f;

      CreateEntityRequest req;
      req.type = turret.type_to_spawn;
      req.position = { offset_pos.x, offset_pos.y, 0 };
      req.velocity = glm::ivec3(-nrm_dir.x * bullet_speed, -nrm_dir.y * bullet_speed, 0);
      r.emplace<CreateEntityRequest>(r.create(), req);

      // request audio
      // r.emplace<AudioRequestPlayEvent>(r.create(), "SHOOT_02");

      // reset timer
      turret.time_between_bullets_left = turret.time_between_bullets;
    }
  }
};

} // namespace game2d