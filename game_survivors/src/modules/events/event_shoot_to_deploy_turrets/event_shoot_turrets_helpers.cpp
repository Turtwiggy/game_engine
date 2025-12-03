#include "pch.hpp"

#include "event_shoot_turrets_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_exploder/actor_exploder_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/systems/system_aoe_slow/aoe_slow_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_particles/components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_sea_turret/weapon_sea_turret_components.hpp"

namespace game2d {

void
give_turret_weapon_def(entt::registry& r, entt::entity wep_e, entt::entity turret_e)
{
  const auto& behaviours = r.get<const WeaponBehaviourComponent>(wep_e).behaviours;

  // add the modifiers to the deployed turret
  const auto& deployer_stats_c = r.get<StatModifierComponent>(wep_e);
  r.emplace_or_replace<StatModifierComponent>(turret_e, deployer_stats_c);

  auto wep_def = get_weapon_def(r, turret_e);
  auto bul_def = get_bullet_def(r, turret_e);

  // note: if the parent turret-deployer has "CHANGE_DAMAGE_TO_ICE"
  // change the damage type spawned by the child spawned turret.
  if (has(behaviours, WeaponBehaviour::CHANGE_DAMAGE_TO_ICE))
    bul_def.damage_type = WEAPON_DAMAGE::ICE;

  r.emplace<WeaponDef>(turret_e, wep_def);
  r.emplace<BulletDef>(turret_e, bul_def);
};

void
spawn_sea_turret(entt::registry& r, entt::entity wep_e, entt::entity player_e)
{
  // deploy a thing!

  const auto& weapons = get_first_component<SINGLE_Weapons>(r);
  const auto& behaviours = r.get<WeaponBehaviourComponent>(wep_e).behaviours;

  // treat turrets as heavy pistols?
  // Note: upgrades apply to both the turret weapon (i.e. sea-turret launcher)
  // and the weapon that is spawned with (e.g. heavy pistol)
  // this works because the upgrades are applied to the player,
  // and adjust e.g. firerate in get_weapon_def & get_bullet_def

  // const auto sea_turret_wep_data = r.get<const Weapon_OnDiskData>(wep_e);
  const auto heavy_pistol_data = weapons.weapons[0]; // todo: dont use idx
  const auto turret_e = spawn_weapon(r, heavy_pistol_data, "weapon_sea_turret");
  // connect_parent_and_weapon(r, player_e, turret_e); // added later

  // offset the turret spawning in a random unit vector.
  static engine::RandomState rnd;
  const auto offset = 10.0f * engine::rand_unit_vector(rnd);
  set_position(r, turret_e, get_position(r, wep_e) + glm::vec2{ offset.x, offset.y });
  set_colour(r, turret_e, r.get<DefaultColour>(player_e).colour);

  // add WeaponDef and BulletDef on the turret
  give_turret_weapon_def(r, wep_e, turret_e);

  // turret-specific components
  r.emplace<EntityTimedLifecycle>(turret_e, 6 * 1000);
  r.emplace<AutofireComponent>(turret_e);
  r.emplace<HasParentComponent>(turret_e, HasParentComponent{ .parent = player_e, .destroy_parent_on_cleanup = false });

  // note: this is basically the "KEEP_DISTANCE" trait
  if (has(behaviours, WeaponBehaviour::TURRET_FOLLOW_PLAYER)) {
    ApplyForceToDynamicTarget tgt_c;
    tgt_c.orbit = true;
    tgt_c.reduce_thrusters = true;
    tgt_c.distance_to_reduce_thrust_meters = 6.0; // meters to shoot from
    r.emplace<ApplyForceToDynamicTarget>(turret_e, tgt_c);
    r.emplace<PhysicsDynamicTarget>(turret_e, player_e);
    b2Body_SetLinearDamping(r.get<const PhysicsBodyComponent>(turret_e).bodyId, 1.0f);
  }

  if (has(behaviours, WeaponBehaviour::TURRET_SLOW_ENEMIES)) {
    r.emplace<AoE_SlowComponent>(turret_e,
                                 AoE_SlowComponent{
                                   .time_between_slow_seconds = 0.5f,
                                   .time_between_slow_seconds_max = 0.5f,
                                 });
  }

  // note: this is basically the "EXPLODE" trait
  if (has(behaviours, WeaponBehaviour::TURRET_EXPLODE_ON_DEATH)) {
    r.get<OnDeathCallbacks>(turret_e).callbacks.clear();

    const std::function<bool(entt::registry&, entt::entity)> filter_criteria = [](entt::registry& r,
                                                                                  entt::entity e) -> bool {
      bool valid_target = false;
      valid_target |= r.try_get<EnemyComponent>(e) != nullptr;
      return valid_target;
    };
    const float enemy_explosion_radius_pixels = 50.0f;
    add_explode_on_death_callback(
      r, turret_e, enemy_explosion_radius_pixels, filter_criteria, ParticleType::DEFAULT_TURRET_EXPLODE);
  }
}

void
handle_shoot_event__deploy_turrets(entt::registry& r, const ShootEvent& evt)
{
  const auto wep_e = evt.weapon_e;

  if (wep_e == entt::null)
    return;

  if (!r.all_of<WeaponSeaTurret>(wep_e))
    return;

  const auto* par_c = r.try_get<HasParentComponent>(wep_e);
  if (!par_c)
    return;
  const auto par_e = par_c->parent;

  const auto wep_def = get_weapon_def(r, wep_e);
  for (int i = 0; i < wep_def.projectiles; i++)
    spawn_sea_turret(r, wep_e, par_e);
}

} // namespace game2d