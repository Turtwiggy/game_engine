#include "autofire_helpers.hpp"

#include "engine/maths/maths.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <magic_enum.hpp>

namespace game2d {

std::vector<float>
generate_angles(float dir, int bullets, float spread_rad)
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
    angles.push_back(clamp_axis(start_angle + i * step));

  return angles;
};

WeaponDef
get_weapon_def(entt::registry& r, entt::entity par_e, entt::entity wep_e)
{
  auto& upgrades_c = r.get<StatModifierComponent>(par_e);

  const auto key_weapon_firerate = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_FIRERATE));
  const auto key_weapon_projectiles = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_PROJECTILES));
  const auto key_weapon_spread = std::string(magic_enum::enum_name(UpgradeableStat::WEAPON_SPREAD));

  const auto val_weapon_firerate = r.get<WeaponFirerate>(wep_e).seconds_between_shots;
  const auto val_weapon_projectiles = r.get<WeaponProjectiles>(wep_e).projectiles;
  const auto val_weapon_spread = r.get<WeaponSpread>(wep_e).angle_between_bullets_deg;

  const auto mod_wep_firerate = upgrades_c.apply_modifiers(val_weapon_firerate, key_weapon_firerate);
  const auto mod_wep_projectiles = (int)upgrades_c.apply_modifiers(val_weapon_projectiles, key_weapon_projectiles);
  const auto mod_wep_spread = (int)upgrades_c.apply_modifiers(val_weapon_spread, key_weapon_spread);

  WeaponDef weapon_def;
  weapon_def.firerate = mod_wep_firerate;
  weapon_def.projectiles = mod_wep_projectiles;
  weapon_def.spread_deg = mod_wep_spread;

  return weapon_def;
};

BulletDef
get_bullet_def(entt::registry& r, entt::entity par_e, entt::entity wep_e)
{
  auto& upgrades_c = r.get<StatModifierComponent>(par_e);
  const auto key_bullet_bounce = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_BOUNCE));
  const auto key_bullet_size = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_SIZE));
  const auto key_bullet_speed = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_SPEED));
  const auto key_bullet_damage = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_DAMAGE));
  const auto key_bullet_pierce = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_PIERCE));
  const auto key_bullet_knockback = std::string(magic_enum::enum_name(UpgradeableStat::BULLET_KNOCKBACK));

  const auto val_bullet_bounce = r.get<BulletBounce>(wep_e).bounces_left;
  const auto val_bullet_size = r.get<BulletSize>(wep_e).size;
  const auto val_bullet_speed = r.get<BulletSpeed>(wep_e).speed;
  const auto val_bullet_damage = r.get<BulletDamage>(wep_e).damage;
  const auto val_bullet_pierce = r.get<BulletPierce>(wep_e).pierce;
  const auto val_bullet_knockback = r.get<BulletKnockback>(wep_e).knockback_force;

  const auto mod_bul_bounce = (int)upgrades_c.apply_modifiers(val_bullet_bounce, key_bullet_bounce);
  const auto mod_bul_size_x = upgrades_c.apply_modifiers(val_bullet_size.x, key_bullet_size); // use x
  const auto mod_bul_speed = (int)upgrades_c.apply_modifiers(val_bullet_speed, key_bullet_speed);
  const auto mod_bul_damage = (int)upgrades_c.apply_modifiers(val_bullet_damage, key_bullet_damage);
  const auto mod_bul_pierce = (int)upgrades_c.apply_modifiers(val_bullet_pierce, key_bullet_pierce);
  const auto mod_bul_knockback = (int)upgrades_c.apply_modifiers(val_bullet_knockback, key_bullet_knockback);

  BulletDef bullet_def;
  bullet_def.key = "bullet_default";
  bullet_def.parent_e = wep_e;
  bullet_def.team = AvailableTeams::player;
  bullet_def.size = { mod_bul_size_x, mod_bul_size_x };
  bullet_def.damage = mod_bul_damage;
  bullet_def.pierce = mod_bul_pierce;
  bullet_def.speed = mod_bul_speed;
  bullet_def.knockback_force = mod_bul_knockback;
  bullet_def.bounces = mod_bul_bounce;
  bullet_def.lifecycle = 3 * 1000;

  auto traits_set = r.get<TraitComponent>(par_e).traits;
  bullet_def.traits = { traits_set.begin(), traits_set.end() };

  return bullet_def;
};

// puts an angle in the range [0, 2π]
float
clamp_axis(float angle)
{
  // range: [-2PI, 2PI]
  angle = std::fmod(angle, engine::TWO_PI);

  // range: [0, 2PI]
  if (angle < 0.0f)
    angle += engine::TWO_PI;

  return angle;
};

// puts an angle in the range [-π, π]
float
normalize_axis(float angle)
{
  angle = std::fmod(angle, engine::TWO_PI);
  if (angle > engine::PI)
    angle -= engine::TWO_PI;
  else if (angle < -engine::PI)
    angle += engine::TWO_PI;
  return angle;
};

float
clamp_angle(float rad_a, float rad_min, float rad_max)
{
  const float max_delta = clamp_axis(rad_max - rad_min) * 0.5;      // 0..π
  const float midpoint = clamp_axis(rad_min + max_delta);           // 0..2π
  const float delta_from_center = normalize_axis(rad_a - midpoint); // -π..π

  if (delta_from_center > max_delta)
    return normalize_axis(midpoint + max_delta);

  if (delta_from_center < -max_delta)
    return normalize_axis(midpoint - max_delta);

  return normalize_axis(rad_a);
};

} // namespace game2d