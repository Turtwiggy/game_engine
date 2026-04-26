#include "enemy_projectile_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"
#include "modules/combat/combat_weapon_type_projectile/combat_weapon_type_projectile_components.hpp"
#include "modules/core/animations/rotate_components.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_cooldown/helpers.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "modules/systems/system_shoot_auto/autofire_helpers.hpp"


namespace game2d {

void
update_enemy_projectile_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
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
    const auto bullet_speed = r.get<BulletSpeed>(e).speed;
    const auto bullet_size = r.get<BulletSize>(e).size;

    const auto wep_e = get_weapons(r, e)[0];
    auto bul_def = get_bullet_def(r, wep_e);

    // modify data for enemy bullet
    bul_def.key = "bullet_archerfish";
    bul_def.size = bullet_size;
    bul_def.team = AvailableTeams::enemy;
    bul_def.damage = 1; // TODO: make enemy bullet correct damage
    bul_def.pierce = 1;
    bul_def.speed = bullet_speed;
    bul_def.lifecycle = 5 * 1000;
    // bullet_def.traits = // no traits for enemies?

    const auto bullet_e = spawn_projectile(r, bul_def, pos);
    // set_colour(r, bullet_e, hex_to_srgb("#00c420"));
    // set_sprite(r, bullet_e, "FIREWORK");

    // set velocity
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    b2Body_SetLinearVelocity(body_c.bodyId, { bullet_speed * nrm_dir.x, bullet_speed * nrm_dir.y });

    // get the enemies bullets to spin
    r.remove<SetTransformRotationBasedOnPhysicsVelocity>(bullet_e);

    AnimationRotate rotate_c{ .speed = 3.0f };
    r.emplace<AnimationRotate>(bullet_e, rotate_c);
  }
}

} // namespace game2d