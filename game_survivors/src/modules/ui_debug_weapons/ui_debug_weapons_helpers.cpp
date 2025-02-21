#include "ui_debug_weapons_helpers.hpp"

#include "magic_enum.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"

#include <format>

namespace game2d {

void
equip_weapon(entt::registry& r, const WeaponData& wep)
{
  // HACK:
  // this shouldnt update ALL components
  // only components on the entity who equipped the weapon
  for (const auto& [key_str, val] : wep.data) {
    const auto key_enum = magic_enum::enum_cast<UpgradeableStat>(key_str);
    if (key_enum == UpgradeableStat::WEAPON_CLIP_SIZE) {
      for (const auto& [e, comp_c] : r.view<WeaponClipSize>().each()) {
        comp_c.bullets_max = val;
      }
    }
    //
    else if (key_enum == UpgradeableStat::WEAPON_PROJECTILES) {
      for (const auto& [e, comp_c] : r.view<WeaponProjectiles>().each()) {
        comp_c.projectiles = val;
      }
    }
    //
    else if (key_enum == UpgradeableStat::WEAPON_FIRERATE) {
      for (const auto& [e, firerate_c] : r.view<WeaponFireRate>().each()) {
        firerate_c.base_firerate = val;
        firerate_c.seconds_between_shots_max = 1.0 / firerate_c.base_firerate;
      }
    }
    //
    else if (key_enum == UpgradeableStat::WEAPON_RELOAD) {
      for (const auto& [e, comp_c] : r.view<WeaponReloadRate>().each()) {
        comp_c.seconds_base_max = val;
      }
    }
    //
    else if (key_enum == UpgradeableStat::WEAPON_RANGE) {
      for (const auto& [e, comp_c] : r.view<WeaponRange>().each()) {
        comp_c.meters = val;
      }
    }
    //
    else if (key_enum == UpgradeableStat::BULLET_DAMAGE) {
      for (const auto& [e, comp_c] : r.view<BulletDamage>().each()) {
        comp_c.damage = val;
      }
    }
    //
    else if (key_enum == UpgradeableStat::BULLET_KNOCKBACK) {
      for (const auto& [e, comp_c] : r.view<BulletKnockback>().each()) {
        comp_c.knockback_force = val;
      }
    }
    //
    else if (key_enum == UpgradeableStat::BULLET_PIERCE) {
      for (const auto& [e, comp_c] : r.view<BulletPierce>().each()) {
        comp_c.pierce = val;
      }
    }
    //
    else {
      const std::string err_str = std::format("Unhandled weapon key: {}", key_str);
      throw std::runtime_error(err_str);
      exit(1);
    }

    // const auto val_weapon_firerate = r.get<WeaponFireRate>(wep_e).base_firerate;
    // const auto val_weapon_projectiles = r.get<WeaponProjectiles>(wep_e).projectiles;
    // const auto val_weapon_spread = r.get<WeaponSpread>(wep_e).angle_between_bullets_deg;
    // const auto val_weapon_clip_size = r.get<WeaponClipSize>(wep_e).bullets_max;
    // const auto val_weapon_reload = r.get<WeaponReloadRate>(wep_e).seconds_base_max;
    // const auto val_weapon_range = r.get<WeaponRange>(wep_e).meters;

    // const auto val_bullet_bounce = r.get<const BulletBounce>(wep_e).bounces_left;
    // const auto val_bullet_size = r.get<const BulletSize>(wep_e).size;
    // const auto val_bullet_speed = r.get<const BulletSpeed>(wep_e).speed;
    // const auto val_bullet_damage = r.get<const BulletDamage>(wep_e).damage;
    // const auto val_bullet_pierce = r.get<const BulletPierce>(wep_e).pierce;
    // const auto val_bullet_knockback = r.get<const BulletKnockback>(wep_e).knockback_force;
    // const auto val_bullet_crit_chance = r.get<const BulletCrit>(wep_e).crit_chance;
    // const auto val_bullet_crit_damage = r.get<const BulletCrit>(wep_e).crit_damage;
    // const auto val_bullet_lifesteal = r.get<const BulletLifesteal>(wep_e).percent_0_100;
  }
};

} // namespace game2d