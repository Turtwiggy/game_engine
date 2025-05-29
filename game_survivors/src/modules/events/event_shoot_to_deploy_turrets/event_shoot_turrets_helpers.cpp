#include "pch.hpp"

#include "event_shoot_turrets_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_exploder/actor_exploder_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_aoe_slow/aoe_slow_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "modules/systems/system_weapon_sea_turret/weapon_sea_turret_components.hpp"

namespace game2d {

void
spawn_sea_turret(entt::registry& r, entt::entity wep_e, entt::entity player_e)
{
  // deploy a thing!

  const auto& weapons = get_first_component<SINGLE_Weapons>(r);

  // treat turrets as heavy pistols?
  // Note: upgrades apply to both the turret weapon (i.e. turret launcher,)
  // todo: validate the weapon that the sea-turret is spawned with (e.g. heavy pistol) has upgrades

  // const auto sea_turret_wep_data = r.get<const Weapon_OnDiskData>(wep_e);
  const auto heavy_pistol_data = weapons.weapons[0]; // todo: dont use idx

  const auto turret_e = spawn_weapon(r, player_e, heavy_pistol_data, "weapon_sea_turret");
  set_position(r, turret_e, get_position(r, wep_e));
  set_colour(r, turret_e, r.get<DefaultColour>(player_e).colour);

  // todo: shouldnt get_bullet_def every time this is called
  auto wep_def = get_weapon_def(r, player_e, turret_e);
  auto bul_def = get_bullet_def(r, player_e, turret_e);

  // note: if the parent turret-deployer has "CHANGE_DAMAGE_TO_ICE"
  // change the damage type spawned by the child spawned turret.
  const auto& behaviours = r.get<WeaponBehaviourComponent>(wep_e).behaviours;
  if (has(behaviours, WeaponBehaviour::CHANGE_DAMAGE_TO_ICE))
    bul_def.damage_type = WEAPON_DAMAGE::ICE;

  r.emplace<WeaponDef>(turret_e, wep_def);
  r.emplace<BulletDef>(turret_e, bul_def);

  // turret-specific components
  r.emplace<EntityTimedLifecycle>(turret_e, 6 * 1000);
  r.emplace<AutofireComponent>(turret_e);
  r.emplace<HasParentComponent>(turret_e, player_e);

  // note: this is basically the "KEEP_DISTANCE" trait
  if (has(behaviours, WeaponBehaviour::TURRET_FOLLOW_PLAYER)) {
    ApplyForceToDynamicTarget tgt_c;
    tgt_c.orbit = true;
    tgt_c.reduce_thrusters = true;
    tgt_c.distance_to_reduce_thrust = 6.0; // meters to shoot from
    r.emplace<ApplyForceToDynamicTarget>(turret_e, tgt_c);
    r.emplace<PhysicsDynamicTarget>(turret_e, player_e);
    r.emplace<ActorSpeedComponent>(turret_e);
    r.get<PhysicsBodyComponent>(turret_e).body->SetLinearDamping(1.0);
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
    add_explode_on_death_callback(r, turret_e, enemy_explosion_radius_pixels, filter_criteria, "death_turret_explode");
  }
}

void
handle_shoot_event__deploy_turrets(entt::registry& r, const ShootEvent& evt)
{
  const auto par_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (par_e == entt::null || wep_e == entt::null)
    return;

  if (!r.all_of<WeaponSeaTurret>(wep_e))
    return;

  spawn_sea_turret(r, wep_e, par_e);
}

} // namespace game2d