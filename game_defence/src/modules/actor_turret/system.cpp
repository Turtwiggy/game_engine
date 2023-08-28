#include "system.hpp"

#include "components.hpp"
#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "physics//components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

namespace game2d {

void
update_turret_system(entt::registry& r, const uint64_t& ms_dt)
{
  const float dt = ms_dt / 1000.0f;
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const float turret_bullet_speed = 20.0f;

  const std::vector<EntityType> valid_types{
    EntityType::actor_spawner,
    EntityType::enemy_grunt,
    EntityType::enemy_shotgunner,
    EntityType::enemy_sniper,
  };

  const auto& turrets = r.view<TurretComponent, TransformComponent, AttackCooldownComponent>();
  for (auto [t_entity, turret, t, cooldown] : turrets.each()) {

    //
    // Get Closest Entity to turret
    //
    const auto info = get_closest(r, t_entity, valid_types);

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

    if (!cooldown.on_cooldown) {

      CreateEntityRequest req;
      req.type = EntityType::bullet_default;
      req.position = { offset_pos.x, offset_pos.y, 0 };
      req.velocity = glm::vec3(-nrm_dir.x * turret_bullet_speed, -nrm_dir.y * turret_bullet_speed, 0);
      r.emplace<CreateEntityRequest>(r.create(), req);

      // request audio
      // r.emplace<AudioRequestPlayEvent>(r.create(), "SHOOT_02");

      reset_cooldown(cooldown);
    }
  }
};

} // namespace game2d