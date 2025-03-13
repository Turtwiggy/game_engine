#include "enemy_projectile_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_projectiles/projectile_helpers.hpp"
#include "modules/core_animations/rotate_components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

void
update_enemy_projectile_system(entt::registry& r)
{
  const auto& view = r.view<const PhysicsDynamicTarget, const ProjectileEnemyComponent, CooldownComponent>();
  for (const auto& [e, target_c, type_c, cooldown_c] : view.each()) {
    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    auto target_e = target_c.target;
    if (target_e == entt::null || !r.valid(target_e))
      continue; // your target died...

    //
    // BULLETS: MAKE EM LARGE AND SLOW
    //

    const auto pos = get_position(r, e);
    const auto raw_dir = get_position(r, target_e) - pos;
    const auto nrm_dir = engine::normalize_safe(raw_dir);

    // int bullet_damage = r.get<BulletDamage>(e).damage;
    int bullet_speed = r.get<BulletSpeed>(e).speed;
    auto bullet_size = r.get<BulletSize>(e).size;

    BulletDef bullet_def(e); // note: not a weapon parent, but an enemy
    bullet_def.key = "bullet_archerfish";
    bullet_def.size = bullet_size;
    bullet_def.team = AvailableTeams::enemy;
    bullet_def.damage = 1; // TODO: make enemy bullet correct damage
    bullet_def.pierce = 1;
    bullet_def.speed = bullet_speed;
    bullet_def.lifecycle = 10 * 1000;
    // bullet_def.traits = // no traits for enemies?

    const auto bullet_e = spawn_projectile(r, bullet_def, pos);
    set_colour(r, bullet_e, hex_to_srgb("#00c420"));
    // set_sprite(r, bullet_e, "FIREWORK");

    // set velocity
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    body_c.body->SetLinearVelocity({ bullet_speed * nrm_dir.x, bullet_speed * nrm_dir.y });

    // get the enemies bullets to spin
    r.remove<SetTransformRotationBasedOnPhysicsVelocity>(bullet_e);

    AnimationRotate rotate_c{ .speed = 3.0f };
    r.emplace<AnimationRotate>(bullet_e, rotate_c);
  }
}

} // namespace game2d