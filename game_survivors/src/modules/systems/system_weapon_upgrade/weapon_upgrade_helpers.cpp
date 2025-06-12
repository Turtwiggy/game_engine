#include "pch.hpp"

#include "weapon_upgrade_helpers.hpp"

#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"

namespace game2d {

WeaponBehaviourMegabulletOut
weapon_behaviour_megabullet(entt::registry& r, const WeaponBehaviourMegabulletIn& in)
{
  WeaponBehaviourMegabulletOut out;
  out.wep_def = in.wep_def;
  out.bul_def = in.bul_def;
  const WeaponDef& wep_def = in.wep_def;
  const BulletDef& bul_def = in.bul_def;

  WeaponDef& altered_w_def = out.wep_def;
  BulletDef& altered_b_def = out.bul_def;

  // merge all bullets in to one huge bullet
  altered_w_def.projectiles = 1;

  // tweak properties of the bullet...
  altered_b_def.size *= wep_def.projectiles;
  altered_b_def.damage *= wep_def.projectiles;
  altered_b_def.pierce *= wep_def.projectiles;
  // altered_b_def.speed =  // dont alter speed?
  // altered_b_def.lifecycle = // dont alter lifecycle?
  altered_b_def.knockback_force *= wep_def.projectiles;
  // altered_b.bullet_def.bounces = ;
  // altered_b.bullet_def.crit_chance = ;
  // altered_b.bullet_def.crit_damage = ;
  // altered_b.bullet_def.lifesteal = ;

  return out;
}

WeaponBehaviourBulletOppositeDirectionOut
weapon_behaviour_shoot_in_opposite_direction(entt::registry& r, const WeaponBehaviourBulletOppositeDirectionIn& in)
{
  const auto& wep_def = in.wep_def;
  const auto& bul_def = in.bul_def;
  const auto& wep_pos = in.wep_pos;
  const auto& angles_rad = in.angles_rad;

  for (int i = 0; i < wep_def.projectiles; i++) {
    const auto bullet_e = spawn_projectile(r, bul_def, wep_pos);
    const auto altered_angle = engine::clamp_axis(angles_rad[i] - engine::PI); // flip the dir
    const auto bullet_dir = engine::angle_radians_to_direction(altered_angle);
    const b2Vec2 bullet_vel = bul_def.speed * b2Vec2{ bullet_dir.x, bullet_dir.y };
    b2Body_SetLinearVelocity(r.get<PhysicsBodyComponent>(bullet_e).bodyId, bullet_vel);
  }

  return {};
}

} // namespace game2d