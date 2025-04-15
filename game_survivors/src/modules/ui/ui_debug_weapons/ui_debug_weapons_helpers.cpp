#include "pch.hpp"

#include "ui_debug_weapons_helpers.hpp"

#include "engine/lifecycle/components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/events/event_damage_lifesteal/lifesteal_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"

namespace game2d {

void
equip_weapon(entt::registry& r, entt::entity e, const Weapon_OnDiskData& wep)
{
  //
  // note: this updates all weapons?
  //
  for (const auto& [key_str, val] : wep.data) {
    const auto key_enum = magic_enum::enum_cast<UpgradeableStat>(key_str);

    for (const auto wep_e : r.get<HasChildrenComponent>(e).children) {
      auto* wep_c = r.try_get<WeaponComponent>(wep_e);
      if (!wep_c)
        continue;

      if (key_enum == UpgradeableStat::WEAPON_CLIP_SIZE) {
        r.get<WeaponClipSize>(wep_e).bullets_max = val;
        r.get<WeaponClipSize>(wep_e).bullets_cur = val;
      }

      else if (key_enum == UpgradeableStat::WEAPON_PROJECTILES)
        r.get<WeaponProjectiles>(wep_e).projectiles = val;

      else if (key_enum == UpgradeableStat::WEAPON_FIRERATE) {
        auto& firerate_c = r.get<WeaponFireRate>(wep_e);
        firerate_c.base_firerate = val;
        firerate_c.seconds_between_shots_max = 1.0 / firerate_c.base_firerate;
      }

      else if (key_enum == UpgradeableStat::WEAPON_RELOAD)
        r.get<WeaponReloadRate>(wep_e).seconds_base_max = val;

      else if (key_enum == UpgradeableStat::WEAPON_RANGE)
        r.get<WeaponRange>(wep_e).meters = val;

      else if (key_enum == UpgradeableStat::WEAPON_SPREAD)
        r.get<WeaponSpread>(wep_e).angle_between_bullets_deg = val;

      //
      // bullet
      //

      else if (key_enum == UpgradeableStat::BULLET_BOUNCE)
        r.get<BulletBounce>(wep_e).bounces_left = val;

      else if (key_enum == UpgradeableStat::BULLET_DAMAGE)
        r.get<BulletDamage>(wep_e).damage = val;

      else if (key_enum == UpgradeableStat::BULLET_KNOCKBACK)
        r.get<BulletKnockback>(wep_e).knockback_force = val;

      else if (key_enum == UpgradeableStat::BULLET_PIERCE)
        r.get<BulletPierce>(wep_e).pierce = val;

      else if (key_enum == UpgradeableStat::BULLET_SIZE)
        r.get<BulletSize>(wep_e).size = { val, val };

      else if (key_enum == UpgradeableStat::BULLET_SPEED)
        r.get<BulletSpeed>(wep_e).speed = val;

      else if (key_enum == UpgradeableStat::BULLET_CRIT_CHANCE)
        r.get<BulletCrit>(wep_e).crit_chance = val;

      else if (key_enum == UpgradeableStat::BULLET_CRIT_DAMAGE)
        r.get<BulletCrit>(wep_e).crit_damage = val;

      else if (key_enum == UpgradeableStat::BULLET_LIFESTEAL)
        r.get<BulletLifesteal>(wep_e).percent_0_100 = val;

      else {
        const std::string err_str = std::format("Unhandled weapon key: {}", key_str);
        throw std::runtime_error(err_str);
        exit(1);
      }
    }
  }
};

} // namespace game2d