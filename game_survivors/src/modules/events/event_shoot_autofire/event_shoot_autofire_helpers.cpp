#include "pch.hpp"

#include "event_shoot_autofire_helpers.hpp"

#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_helpers.hpp"

namespace game2d {

void
handle_shoot_event__autofire(entt::registry& r, const ShootEvent& evt)
{
  const auto par_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (par_e == entt::null || wep_e == entt::null)
    return;

  if (!r.all_of<AutofireComponent>(wep_e))
    return;

  const auto& wep_t = r.get<const TransformComponent>(wep_e);
  const auto wep_pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
  const auto& wep_def = r.get<const WeaponDef>(wep_e);
  const auto& wep_autofire = r.get<const AutofireComponent>(wep_e);
  const auto& wep_behaviours_c = r.get<const WeaponBehaviourComponent>(wep_e);

  const auto bul_def = r.get<const BulletDef>(wep_e);
  WeaponDef altered_w_def = wep_def;
  BulletDef altered_b_def = bul_def;

  // Double Projectiles?
  if (has(wep_behaviours_c.behaviours, WeaponBehaviour::DOUBLE_PROJECTILES))
    altered_w_def.projectiles *= 2.0f;

  // Merge all bullets in to one mega bullet?
  if (has(wep_behaviours_c.behaviours, WeaponBehaviour::MEGABULLET)) {
    const auto in = WeaponBehaviourMegabulletIn{ .wep_def = wep_def, .bul_def = bul_def };
    const auto out = weapon_behaviour_megabullet(r, in);
    altered_w_def = out.wep_def;
    altered_b_def = out.bul_def;
  }

  // using directly from transform is pretty sketch
  const float shoot_angle = wep_t.rotation_radians.z;

  // Spawn X amount of bullets
  // Note: even though the angle that the weapon can fire at is limited (e.g. 30 degrees)
  // If the weapon has enough weapon spread (e.g. 90 degrees)
  // It could still shoot at the limited angles.
  const auto par_vel_meters = r.get<PhysicsBodyComponent>(par_e).body->GetLinearVelocity();
  const auto spread_rad = altered_w_def.spread_deg * engine::Deg2Rad;
  const auto ar = generate_angles(shoot_angle, altered_w_def.projectiles, spread_rad);
  for (int i = 0; i < altered_w_def.projectiles; i++) {
    const auto bullet_e = spawn_projectile(r, altered_b_def, wep_pos);
    const auto bullet_dir = engine::normalize_safe(engine::angle_radians_to_direction(ar[i]));
    const auto bullet_vel = altered_b_def.speed * b2Vec2{ bullet_dir.x, bullet_dir.y };
    r.get<PhysicsBodyComponent>(bullet_e).body->SetLinearVelocity(bullet_vel);
  }

  // shoot bullets in opposite direction?
  if (has(wep_behaviours_c.behaviours, WeaponBehaviour::SHOOT_BULLETS_OPPOSITE_DIRECTION)) {
    const auto in = WeaponBehaviourBulletOppositeDirectionIn{
      .wep_def = altered_w_def,
      .bul_def = altered_b_def,
      .wep_pos = wep_pos,
      .angles_rad = ar,
    };
    weapon_behaviour_shoot_in_opposite_direction(r, in);
  }
}

} // namespace game2d