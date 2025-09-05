#include "pch.hpp"

#include "autofire_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"
#include "modules/combat/combat_weapon_type_area/combat_weapon_type_area_components.hpp"
#include "modules/combat/combat_weapon_type_projectile/combat_weapon_type_projectile_components.hpp"
#include "modules/events/event_damage_lifesteal/lifesteal_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"

namespace game2d {

std::vector<float>
generate_angles(const float dir, const int bullets, const float spread_rad)
{
  if (bullets == 0)
    return {};
  if (bullets == 1)
    return { dir };

  std::vector<float> angles;

  float step = spread_rad;

  // Note: shouldnt divide step by (bullets/1) for odd number of bullets.
  // e.g. if bullets = 2, spread = 90, step would be 90/(2-1)=90, which is correct
  // e.g. if bullets = 3, spread = 360, step would be 360/(3-1)=180, which is wrong
  bool is_even = bullets % 2 == 0;
  step /= is_even ? (bullets - 1) : bullets;

  const float start_angle = dir - (spread_rad / 2.0f);

  for (int i = 0; i < bullets; i++)
    angles.push_back(engine::clamp_axis(start_angle + i * step));

  return angles;
};

Weapon_OnDiskData
get_weapon_data(entt::registry& r, const std::string weapon_key)
{
  const auto& weps_c = get_first_component<SINGLE_Weapons>(r);

  auto get_key = []<typename T>(const std::vector<T>& data, const std::string& key) -> std::optional<T> {
    const auto it = std::find_if(data.begin(), data.end(), [&key](const T& item) { return item.key == key; });
    if (it == data.end())
      return std::nullopt;
    return (*it);
  };
  const Weapon_OnDiskData data = get_key(weps_c.weapons, weapon_key).value();
  return data;
}

WeaponDef
get_weapon_def(entt::registry& r, const entt::entity wep_e)
{
  auto& upgrades_c = r.get<const StatModifierComponent>(wep_e);

  const auto key_weapon_firerate = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_FIRERATE));
  const auto key_weapon_projectiles = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_PROJECTILES));
  const auto key_weapon_spread = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_SPREAD));
  const auto key_weapon_clip_size = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_CLIP_SIZE));
  const auto key_weapon_reload = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_RELOAD));
  const auto key_weapon_range = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_RANGE));

  const auto val_weapon_firerate = r.get<const WeaponFireRate>(wep_e).base_firerate;
  const auto val_weapon_projectiles = r.get<const WeaponProjectiles>(wep_e).projectiles;
  const auto val_weapon_spread = r.get<const WeaponSpread>(wep_e).angle_between_bullets_deg;
  const auto val_weapon_clip_size = r.get<const WeaponClipSize>(wep_e).bullets_max;
  const auto val_weapon_reload = r.get<const WeaponReloadRate>(wep_e).seconds_base_max;
  const auto val_weapon_range = r.get<const WeaponRange>(wep_e).meters;

  const auto mod_wep_firerate = upgrades_c.apply_modifiers(val_weapon_firerate, key_weapon_firerate);
  const auto mod_wep_projectiles = (int)upgrades_c.apply_modifiers(val_weapon_projectiles, key_weapon_projectiles);
  const auto mod_wep_spread = (int)upgrades_c.apply_modifiers(val_weapon_spread, key_weapon_spread);
  const auto mod_wep_clip_size = upgrades_c.apply_modifiers(val_weapon_clip_size, key_weapon_clip_size);
  const auto mod_wep_reload = upgrades_c.apply_modifiers(val_weapon_reload, key_weapon_reload);
  const auto mod_wep_range = upgrades_c.apply_modifiers(val_weapon_range, key_weapon_range);

  WeaponDef weapon_def;
  weapon_def.fire_rate = mod_wep_firerate;
  weapon_def.projectiles = mod_wep_projectiles;
  weapon_def.spread_deg = mod_wep_spread;
  weapon_def.bullets_max = mod_wep_clip_size;
  weapon_def.reload_rate = mod_wep_reload;
  weapon_def.range = mod_wep_range;
  return weapon_def;
};

BulletDef
get_bullet_def(entt::registry& r, const entt::entity wep_e)
{
  if (wep_e == entt::null) {
    SDL_Log("Error creating BulletDef");
    exit(1); // crash
  }

  const auto& upgrades_c = r.get<const StatModifierComponent>(wep_e);

  const auto key_bullet_bounce = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_BOUNCE));
  const auto key_bullet_size = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_SIZE));
  const auto key_bullet_speed = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_SPEED));
  const auto key_bullet_damage = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_DAMAGE));
  const auto key_bullet_pierce = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_PIERCE));
  const auto key_bullet_knockback = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_KNOCKBACK));
  const auto key_bullet_crit_chance = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_CRIT_CHANCE));
  const auto key_bullet_crit_damage = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_CRIT_DAMAGE));
  const auto key_bullet_lifesteal = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_LIFESTEAL));
  const auto key_bullet_liftime = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_LIFETIME));

  const auto val_bullet_bounce = r.get<const BulletBounce>(wep_e).bounces_left;
  const auto val_bullet_size = r.get<const BulletSize>(wep_e).size;
  const auto val_bullet_speed = r.get<const BulletSpeed>(wep_e).speed;
  const auto val_bullet_damage = r.get<const BulletDamage>(wep_e).damage;
  const auto val_bullet_pierce = r.get<const BulletPierce>(wep_e).pierce;
  const auto val_bullet_knockback = r.get<const BulletKnockback>(wep_e).knockback_force;
  const auto val_bullet_crit_chance = r.get<const BulletCrit>(wep_e).crit_chance;
  const auto val_bullet_crit_damage = r.get<const BulletCrit>(wep_e).crit_damage;
  const auto val_bullet_lifesteal = r.get<const BulletLifesteal>(wep_e).percent_0_100;
  const auto val_bullet_lifetime_ms = r.get<const BulletLifetime>(wep_e).seconds * 1000;

  const auto mod_bul_bounce = (int)upgrades_c.apply_modifiers(val_bullet_bounce, key_bullet_bounce);
  const auto mod_bul_size_x = upgrades_c.apply_modifiers(val_bullet_size.x, key_bullet_size);
  const auto mod_bul_size_y = upgrades_c.apply_modifiers(val_bullet_size.y, key_bullet_size);
  const auto mod_bul_speed = upgrades_c.apply_modifiers(val_bullet_speed, key_bullet_speed);
  const auto mod_bul_damage = (int)upgrades_c.apply_modifiers(val_bullet_damage, key_bullet_damage);
  const auto mod_bul_pierce = (int)upgrades_c.apply_modifiers(val_bullet_pierce, key_bullet_pierce);
  const auto mod_bul_knockback = (int)upgrades_c.apply_modifiers(val_bullet_knockback, key_bullet_knockback);
  const auto mod_bul_crit_chance = upgrades_c.apply_modifiers(val_bullet_crit_chance, key_bullet_crit_chance);
  const auto mod_bul_crit_damage = upgrades_c.apply_modifiers(val_bullet_crit_damage, key_bullet_crit_damage);
  const auto mod_bul_lifesteal = upgrades_c.apply_modifiers(val_bullet_lifesteal, key_bullet_lifesteal);
  const auto mod_bul_lifetime_ms = upgrades_c.apply_modifiers(val_bullet_lifetime_ms, key_bullet_liftime);

  BulletDef bullet_def;
  bullet_def.key = "bullet_default";
  bullet_def.parent_e = wep_e;
  bullet_def.team = AvailableTeams::player;
  bullet_def.size = { mod_bul_size_x, mod_bul_size_y };
  bullet_def.damage = mod_bul_damage;
  bullet_def.pierce = mod_bul_pierce;
  bullet_def.speed = mod_bul_speed;
  bullet_def.knockback_force = mod_bul_knockback;
  bullet_def.bounces = mod_bul_bounce;
  bullet_def.crit_chance = mod_bul_crit_chance;
  bullet_def.crit_damage = mod_bul_crit_damage;
  bullet_def.lifesteal = mod_bul_lifesteal;
  bullet_def.lifecycle = mod_bul_lifetime_ms;

  auto behaviours_set = r.get<WeaponBehaviourComponent>(wep_e).behaviours;
  bullet_def.wep_behaviours = { behaviours_set.begin(), behaviours_set.end() };

  auto damage_type = r.get<WeaponDamageTypeComponent>(wep_e).type;
  if (has(behaviours_set, WeaponBehaviour::CHANGE_DAMAGE_TO_FIRE))
    damage_type = WEAPON_DAMAGE::FIRE;
  if (has(behaviours_set, WeaponBehaviour::CHANGE_DAMAGE_TO_ICE))
    damage_type = WEAPON_DAMAGE::ICE;
  if (has(behaviours_set, WeaponBehaviour::CHANGE_DAMAGE_TO_POISON))
    damage_type = WEAPON_DAMAGE::POISON;
  if (has(behaviours_set, WeaponBehaviour::CHANGE_DAMAGE_TO_SHOCK))
    damage_type = WEAPON_DAMAGE::SHOCK;
  bullet_def.damage_type = damage_type;

  return bullet_def;
};

AreaDef_ModifiersApplied
get_area_def(entt::registry& r, const entt::entity wep_e)
{
  if (wep_e == entt::null) {
    SDL_Log("Error creating AreaDef");
    exit(1); // crash
  }
  const auto& upgrades_c = r.get<const StatModifierComponent>(wep_e);

  const auto key_beams_per_weapon = std::string(magic_enum::enum_name(UpgradeableStat::AREA_BEAMS_PER_WEAPON));
  const auto key_size = std::string(magic_enum::enum_name(UpgradeableStat::AREA_SIZE));
  const auto key_stack_damage = std::string(magic_enum::enum_name(UpgradeableStat::AREA_STACK_DAMAGE));
  const auto key_stack_duration = std::string(magic_enum::enum_name(UpgradeableStat::AREA_STACK_DURATION));
  const auto key_stacks_applied = std::string(magic_enum::enum_name(UpgradeableStat::AREA_STACKS_PER_SHOT));

  const auto val_beams_per_weapon = r.get<const AreaWeapon_Beams>(wep_e).beams;
  const auto val_size_x = r.get<const AreaWeapon_Size>(wep_e).size_x;
  const auto val_size_y = r.get<const AreaWeapon_Size>(wep_e).size_y;
  const auto val_stack_damage = r.get<const AreaWeapon_StackDamage>(wep_e).damage_mul;
  const auto val_stack_duration = r.get<const AreaWeapon_StackDuration>(wep_e).seconds;
  const auto val_stacks_applied = r.get<const AreaWeapon_StacksAppliedPerShot>(wep_e).stacks;

  const auto mod_beams_per_weapon = (int)upgrades_c.apply_modifiers(val_beams_per_weapon, key_beams_per_weapon);
  const auto mod_size_x = upgrades_c.apply_modifiers(val_size_x, key_size);
  const auto mod_size_y = upgrades_c.apply_modifiers(val_size_y, key_size);
  const auto mod_stack_damage = upgrades_c.apply_modifiers(val_stack_damage, key_stack_damage);
  const auto mod_stack_duration = upgrades_c.apply_modifiers(val_stack_duration, key_stack_duration);
  const auto mod_stacks_applied = (int)upgrades_c.apply_modifiers(val_stacks_applied, key_stacks_applied);

  return AreaDef_ModifiersApplied{
    .beams = mod_beams_per_weapon,
    .size_x = mod_size_x,
    .size_y = mod_size_y,
    .stack_damage = mod_stack_damage,
    .stack_duration = mod_stack_duration,
    .stacks_per_shot = mod_stacks_applied,
  };
}

} // namespace game2d