#include "trait_fanfire_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "resources/data.hpp"

namespace game2d {

struct FanfireTraitComponent
{
  int shots_until_fanfire = 10;
  int shots_until_fanfire_left = 10;

  int projectiles_to_fanfire = 10;
};

void
handle_shoot_event__trait_fanfire(entt::registry& r, const ShootEvent& evt)
{
  const auto wep_e = evt.weapon_e;
  if (wep_e == entt::null)
    return;

  const auto* trait_c = r.try_get<WeaponBehaviourComponent>(wep_e);
  if (!trait_c)
    return;

  const auto behaviour = WeaponBehaviour::SHOOT_FANFIRE_ON_LAST_BULLET;
  if (!has(trait_c->behaviours, behaviour))
    return;

  const auto& wep_def = r.get<WeaponDef>(wep_e);
  auto& fanfire_c = r.get_or_emplace<FanfireTraitComponent>(wep_e);
  fanfire_c.shots_until_fanfire = wep_def.bullets_max;
  fanfire_c.shots_until_fanfire_left -= 1;

  // activation: based on number of shots fired
  if (fanfire_c.shots_until_fanfire_left > 0)
    return;
  fanfire_c.shots_until_fanfire_left = fanfire_c.shots_until_fanfire;

  // do the fanfire
  // Note: count the bullet as one of the player's bullets.
  BulletDef bul_def = r.get<BulletDef>(wep_e);
  bul_def.damage *= 0.2;   // deal 20% damage
  bul_def.size = { 4, 4 }; // fanfire bullets slightly smaller
  bul_def.damage_type = WEAPON_DAMAGE::KINETIC;

  auto col = my_white;
  if (auto* parent_c = r.try_get<HasParentComponent>(wep_e)) {
    const auto par_e = parent_c->parent;
    const auto& player_col = r.get<DefaultColour>(par_e).colour;
    col = player_col;
  }

  const auto angles_rad = generate_angles(0, fanfire_c.projectiles_to_fanfire, engine::TWO_PI);
  auto pos = get_position(r, wep_e);
  for (const auto& a : angles_rad) {

    const auto bullet_e = spawn_projectile(r, bul_def, pos);
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    const auto bullet_dir = engine::angle_radians_to_direction(a);
    const auto bullet_vel = bul_def.speed * b2Vec2{ bullet_dir.x, bullet_dir.y };
    b2Body_SetLinearVelocity(body_c.bodyId, bullet_vel);

    // fanfire bullets to player col
    set_colour(r, bullet_e, col);
  }
}

} // namespace game2d