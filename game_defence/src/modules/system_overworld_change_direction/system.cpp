#include "system.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_overworld_change_direction_system(entt::registry& r)
{
  static engine::RandomState rnd;

  const auto& view = r.view<ApplyForceInDirectionComponent, AttackCooldownComponent, const PhysicsBodyComponent>();
  for (const auto& [e, dir_c, cooldown_c, body_c] : view.each()) {
    if (cooldown_c.on_cooldown)
      continue; // put on cooldown
    cooldown_c.on_cooldown = true;
    cooldown_c.time_between_attack_left = cooldown_c.time_between_attack;

    const float speed = 25;

    // generate an angle +- 45 degrees from current heading
    const float radians_current = body_c.body->GetAngle();
    const float radians_adjustment = 45.0f * engine::Deg2Rad;
    const float radians_min = radians_current - radians_adjustment;
    const float radians_max = radians_current + radians_adjustment;

    const float rnd_angle = engine::rand_det_s(rnd.rng, radians_min, radians_max);
    const auto dir = engine::angle_radians_to_direction(rnd_angle);
    dir_c.tgt_vel = { speed * dir.x, speed * dir.y };
  }
}

} // namespace game2d