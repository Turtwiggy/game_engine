#include "enemy_projectile_helpers.hpp"

#include "modules/actor_enemy/components.hpp"
#include "modules/event_coll_bullet_enemy/event_coll_bullet_enemy_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"

namespace game2d {

void
add_projectile_enemy_components(entt::registry& r, entt::entity e)
{
  r.emplace<ProjectileEnemyComponent>(e);

  ApplyForceToDynamicTarget tgt_c;
  tgt_c.orbit = true;
  tgt_c.reduce_thrusters = true;
  tgt_c.speed = 100.0f;
  tgt_c.distance_to_reduce_thrust = 600; // distance to shoot from
  r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);

  // time between shots
  CooldownComponent cooldown_c;
  cooldown_c.time_max = 2.0f;
  cooldown_c.time = 0.0f;
  r.emplace<CooldownComponent>(e, cooldown_c);
  r.emplace<BulletDamage>(e, BulletDamage{ 1 });
}

} // namespace game2d