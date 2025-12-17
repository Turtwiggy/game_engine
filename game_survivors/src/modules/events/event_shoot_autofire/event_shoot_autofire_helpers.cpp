#include "pch.hpp"

#include "event_shoot_autofire_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_flamethrower/flamethrower_components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_weapon_sea_turret/weapon_sea_turret_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_helpers.hpp"

namespace game2d {

void
handle_shoot_event__autofire(entt::registry& r, const ShootEvent& evt)
{
  const auto wep_e = evt.weapon_e;

  if (wep_e == entt::null || !r.valid(wep_e))
    return;

  if (!r.all_of<AutofireComponent, BulletDef>(wep_e))
    return;

  if (r.all_of<WeaponSeaTurret>(wep_e))
    return; // the sea-turret itself should not shoot

  const auto& wep_t = r.get<const TransformComponent>(wep_e);
  const auto wep_pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
  const auto& wep_autofire = r.get<const AutofireComponent>(wep_e);
  const auto& wep_behaviours_c = r.get<const WeaponBehaviourComponent>(wep_e);

  const auto wep_def = get_weapon_def(r, wep_e);
  const auto bul_def = get_bullet_def(r, wep_e);
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

  // const auto par_id = r.get<const PhysicsBodyComponent>(par_e).bodyId;
  // const auto vel_meters = b2Body_GetLinearVelocity(par_id);

  const auto& wep_data = r.get<Weapon_OnDiskData>(wep_e);
  bool is_fire = wep_data.damage_as_enum == WEAPON_DAMAGE::FIRE;
  is_fire |= has(wep_behaviours_c.behaviours, WeaponBehaviour::CHANGE_DAMAGE_TO_FIRE);

  const auto spread_rad = altered_w_def.spread_deg * engine::Deg2Rad;
  const auto ar = generate_angles(shoot_angle, altered_w_def.projectiles, spread_rad);
  for (int i = 0; i < altered_w_def.projectiles; i++) {
    const auto bullet_e = spawn_projectile(r, altered_b_def, wep_pos);
    set_rotation(r, bullet_e, shoot_angle);

    if (is_fire) {
      r.remove<SpriteComponent>(bullet_e);
      r.emplace<FlamethrowerFlameComponent>(bullet_e);
      auto fixture_e = get_fixture_by_tag(r, bullet_e, "fixture_bullet");
      r.emplace<FlamethrowerFlameFixtureComponent>(fixture_e);
    }

    const auto bullet_dir = engine::normalize_safe(engine::angle_radians_to_direction(ar[i]));
    const auto bullet_vel = altered_b_def.speed * b2Vec2{ bullet_dir.x, bullet_dir.y };
    b2Body_SetLinearVelocity(r.get<PhysicsBodyComponent>(bullet_e).bodyId, bullet_vel);
  }

  // shoot bullets in opposite direction?
  if (has(wep_behaviours_c.behaviours, WeaponBehaviour::SHOOT_BULLETS_OPPOSITE_DIRECTION)) {
    const auto in = WeaponBehaviourBulletOppositeDirectionIn{
      .wep_e = wep_e,
      .wep_def = altered_w_def,
      .bul_def = altered_b_def,
      .wep_pos = wep_pos,
      .angles_rad = ar,
    };
    weapon_behaviour_shoot_in_opposite_direction(r, in);
  }
}

} // namespace game2d