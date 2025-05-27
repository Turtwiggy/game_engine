#include "pch.hpp"

#include "autofire_helpers.hpp"

#include "engine/maths/maths.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
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

WeaponDef
get_weapon_def(entt::registry& r, const entt::entity player_e, const entt::entity wep_e)
{
  auto& upgrades_c = r.get<const StatModifierComponent>(player_e);

  const auto key_weapon_firerate = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_FIRERATE));
  const auto key_weapon_projectiles = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_PROJECTILES));
  const auto key_weapon_spread = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_SPREAD));
  const auto key_weapon_clip_size = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_CLIP_SIZE));
  const auto key_weapon_reload = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_RELOAD));
  const auto key_weapon_range = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_RANGE));

  const auto val_weapon_firerate = r.get<WeaponFireRate>(wep_e).base_firerate;
  const auto val_weapon_projectiles = r.get<WeaponProjectiles>(wep_e).projectiles;
  const auto val_weapon_spread = r.get<WeaponSpread>(wep_e).angle_between_bullets_deg;
  const auto val_weapon_clip_size = r.get<WeaponClipSize>(wep_e).bullets_max;
  const auto val_weapon_reload = r.get<WeaponReloadRate>(wep_e).seconds_base_max;
  const auto val_weapon_range = r.get<WeaponRange>(wep_e).meters;

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
get_bullet_def(entt::registry& r, const entt::entity player_e, const entt::entity wep_e)
{
  const auto& upgrades_c = r.get<const StatModifierComponent>(player_e);

  const auto key_bullet_bounce = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_BOUNCE));
  const auto key_bullet_size = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_SIZE));
  const auto key_bullet_speed = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_SPEED));
  const auto key_bullet_damage = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_DAMAGE));
  const auto key_bullet_pierce = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_PIERCE));
  const auto key_bullet_knockback = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_KNOCKBACK));
  const auto key_bullet_crit_chance = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_CRIT_CHANCE));
  const auto key_bullet_crit_damage = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_CRIT_DAMAGE));
  const auto key_bullet_lifesteal = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_LIFESTEAL));

  const auto val_bullet_bounce = r.get<const BulletBounce>(wep_e).bounces_left;
  const auto val_bullet_size = r.get<const BulletSize>(wep_e).size;
  const auto val_bullet_speed = r.get<const BulletSpeed>(wep_e).speed;
  const auto val_bullet_damage = r.get<const BulletDamage>(wep_e).damage;
  const auto val_bullet_pierce = r.get<const BulletPierce>(wep_e).pierce;
  const auto val_bullet_knockback = r.get<const BulletKnockback>(wep_e).knockback_force;
  const auto val_bullet_crit_chance = r.get<const BulletCrit>(wep_e).crit_chance;
  const auto val_bullet_crit_damage = r.get<const BulletCrit>(wep_e).crit_damage;
  const auto val_bullet_lifesteal = r.get<const BulletLifesteal>(wep_e).percent_0_100;

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

  if (wep_e == entt::null || player_e == entt::null) {
    SDL_Log("Error creating BulletDef; invalid parents");
    exit(1); // crash
  }

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
  bullet_def.lifecycle = 3 * 1000;
  bullet_def.crit_chance = mod_bul_crit_chance;
  bullet_def.crit_damage = mod_bul_crit_damage;
  bullet_def.lifesteal = mod_bul_lifesteal;

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

} // namespace game2d