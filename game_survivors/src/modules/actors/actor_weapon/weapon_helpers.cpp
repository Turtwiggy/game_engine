#include "pch.hpp"

#include "weapon_components.hpp"
#include "weapon_helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"
#include "modules/combat/combat_weapon_type_area/combat_weapon_type_area_components.hpp"
#include "modules/combat/combat_weapon_type_projectile/combat_weapon_type_projectile_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/raws/raws_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/events/event_damage_lifesteal/lifesteal_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_sea_turret/weapon_sea_turret_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"

namespace game2d {

entt::entity
spawn_weapon(entt::registry& r, const entt::entity player_e, const Weapon_OnDiskData& w_data, std::string key)
{
  glm::vec2 weapon_size = { 6, 3 };

  const auto wep_type_enum = w_data.type_as_enum;
  const auto wep_e = spawn(r, key);
  give_life(r, wep_e, { 0, 0 }, weapon_size);
  r.emplace<TeamComponent>(wep_e, TeamComponent{ AvailableTeams::player });

  // weapon stats
  r.emplace<WeaponComponent>(wep_e);
  r.emplace<WeaponLevelComponent>(wep_e);
  r.emplace<WeaponDamageTypeComponent>(wep_e, w_data.damage_as_enum);
  r.emplace<WeaponBehaviourComponent>(wep_e);
  r.emplace<Weapon_OnDiskData>(wep_e, w_data);
  r.emplace<StatModifierComponent>(wep_e); // BULLET_ and WEAPON_ stats (per weapon)

  const auto get_or_default = [&](std::string key, float def) -> float {
    if (w_data.data.contains(key))
      return w_data.data.at(key);
    return def;
  };

  const float WEAPON_CLIP_SIZE = get_or_default("WEAPON_CLIP_SIZE", 1);     // bullets per clip
  const float WEAPON_PROJECTILES = get_or_default("WEAPON_PROJECTILES", 1); // bullets shot per shoot event
  const float WEAPON_FIRERATE = get_or_default("WEAPON_FIRERATE", 2.0f);    // shots per second
  const float WEAPON_RELOAD = get_or_default("WEAPON_RELOAD", 0.5f);        // seconds to reload
  const float WEAPON_RANGE = get_or_default("WEAPON_RANGE", 3.0f);          // meters
  const float WEAPON_SPREAD = get_or_default("WEAPON_SPREAD", 30.0f);       // degrees between shots

  const float BULLET_BOUNCE = get_or_default("BULLET_BOUNCE", 0.0f);
  const float BULLET_DAMAGE = get_or_default("BULLET_DAMAGE", 10.0f);
  const float BULLET_KNOCKBACK = get_or_default("BULLET_KNOCKBACK", 0.0f);
  const float BULLET_PIERCE = get_or_default("BULLET_PIERCE", 1.0f);
  const float BULLET_SIZE = get_or_default("BULLET_SIZE", 6.0f);
  const float BULLET_SPEED = get_or_default("BULLET_SPEED", 1.5f);               // meters
  const float BULLET_CRIT_CHANCE = get_or_default("BULLET_CRIT_CHANCE", 0.0f);   // 0-100
  const float BULLET_CRIT_DAMAGE = get_or_default("BULLET_CRIT_DAMAGE", 150.0f); // 100%+
  const float BULLET_LIFESTEAL = get_or_default("BULLET_LIFESTEAL", 0.0f);       // 0-100% of your bullet damage
  const float BULLET_LIFETIME = get_or_default("BULLET_LIFETIME", 3.0f);

  const float AREA_BEAMS_PER_WEAPON = get_or_default("AREA_BEAMS_PER_WEAPON", 1);
  const float AREA_SIZE_X = get_or_default("AREA_SIZE_X", 1);
  const float AREA_SIZE_Y = get_or_default("AREA_SIZE_Y", 1);
  const float AREA_STACK_DAMAGE = get_or_default("AREA_STACK_DAMAGE", 1);
  const float AREA_STACK_DURATION = get_or_default("AREA_STACK_DURATION", 0.5f);
  const float AREA_STACKS_APPLIED_PER_SHOT = get_or_default("AREA_STACKS_APPLIED_PER_SHOT", 1);

  // load weapons from config
  const auto fr_c = WeaponFireRate{ .base_firerate = WEAPON_FIRERATE, .seconds_between_shots_max = 1.0f / WEAPON_FIRERATE };
  r.emplace<WeaponSpread>(wep_e, WeaponSpread{ WEAPON_SPREAD });
  r.emplace<WeaponProjectiles>(wep_e, WeaponProjectiles{ .projectiles = (int)WEAPON_PROJECTILES });
  r.emplace<WeaponClipSize>(wep_e, WeaponClipSize{ .bullets_max = (int)WEAPON_CLIP_SIZE });
  r.emplace<WeaponFireRate>(wep_e, fr_c);
  r.emplace<WeaponReloadRate>(wep_e, WeaponReloadRate{ .seconds_base_max = WEAPON_RELOAD });
  r.emplace<WeaponRange>(wep_e, WeaponRange{ .meters = WEAPON_RANGE });

  // bullets that the weapon fires
  if (wep_type_enum == WEAPON_TYPE::PROJECTILE || wep_type_enum == WEAPON_TYPE::DEPLOY) {
    r.emplace<BulletBounce>(wep_e, BulletBounce{ (int)BULLET_BOUNCE }); // 0 bounce by default
    r.emplace<BulletDamage>(wep_e, BulletDamage{ (int)BULLET_DAMAGE });
    r.emplace<BulletPierce>(wep_e, BulletPierce{ (int)BULLET_PIERCE });
    r.emplace<BulletSize>(wep_e, BulletSize{ { BULLET_SIZE, BULLET_SIZE } });
    r.emplace<BulletSpeed>(wep_e, BulletSpeed{ BULLET_SPEED });
    r.emplace<BulletKnockback>(wep_e, BulletKnockback{ BULLET_KNOCKBACK });
    r.emplace<BulletCrit>(wep_e, BulletCrit{ .crit_chance = BULLET_CRIT_CHANCE, .crit_damage = BULLET_CRIT_DAMAGE });
    r.emplace<BulletLifesteal>(wep_e, BulletLifesteal{ .percent_0_100 = BULLET_LIFESTEAL });
    r.emplace<BulletLifetime>(wep_e, BulletLifetime{ .seconds = BULLET_LIFETIME });
  }

  // e.g. sea-turrets
  // if (wep_type_enum == WEAPON_TYPE::DEPLOY)
  //   r.emplace<WeaponSeaTurret>(wep_e);

  // e.g. flamethrower
  if (wep_type_enum == WEAPON_TYPE::AREA) {
    r.emplace<AreaWeapon_Beams>(wep_e, AreaWeapon_Beams{ (int)AREA_BEAMS_PER_WEAPON });
    r.emplace<AreaWeapon_Size>(wep_e, AreaWeapon_Size{ AREA_SIZE_X, AREA_SIZE_Y });
    r.emplace<AreaWeapon_StackDamage>(wep_e, AreaWeapon_StackDamage{ AREA_STACK_DAMAGE });
    r.emplace<AreaWeapon_StackDuration>(wep_e, AreaWeapon_StackDuration{ AREA_STACK_DURATION });
    r.emplace<AreaWeapon_StacksAppliedPerShot>(wep_e, AreaWeapon_StacksAppliedPerShot{ (int)AREA_STACKS_APPLIED_PER_SHOT });
  }

  set_z_index(r, wep_e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);
  return wep_e;
};

SINGLE_Weapons
load_weapons(entt::registry& r, std::string filepath)
{
  SDL_Log("loading weapons... %s", filepath.c_str());

  // load from disk
  std::ifstream t(filepath);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);

  // validate weapon type
  auto weapons_c = root.get<SINGLE_Weapons>();
  for (auto& weapon : weapons_c.weapons) {
    weapon.type_as_enum = magic_enum::enum_cast<WEAPON_TYPE>(weapon.weapon_type).value();
    weapon.damage_as_enum = magic_enum::enum_cast<WEAPON_DAMAGE>(weapon.weapon_damage).value();

    // validate audio file exists.
    const auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);
    for (const auto& audio_tag : weapon.audio) {
      const auto matching_sound = [&](const Sound& s) { return s.tag == audio_tag; };
      auto it = std::find_if(audio_c.sounds.begin(), audio_c.sounds.end(), matching_sound);
      if (it == audio_c.sounds.end()) {
        const auto err = std::format("missing sound {} declared in weapons.json. weapon: {}", audio_tag, weapon.key);
        throw std::runtime_error(err);
      }
    }
  }

  // validate weapon upgrades.
  for (const auto& weapon_upgrade : weapons_c.weapon_upgrades) {

    // validate WeaponBehaviour.
    if (weapon_upgrade.wb_key != "")
      const auto wb = magic_enum::enum_cast<WeaponBehaviour>(weapon_upgrade.wb_key).value();

    // validate UpgradeableStat.
    for (const auto& stat : weapon_upgrade.stats) {
      if (stat.stat != "")
        const auto us = magic_enum::enum_cast<UpgradeableStat>(stat.stat).value();
    }
  }

  return weapons_c;
};

std::vector<entt::entity>
get_weapons(entt::registry& r, entt::entity player_e)
{
  const auto* child_c = r.try_get<HasChildrenComponent>(player_e);
  if (!child_c)
    return {};

  std::vector<entt::entity> weapons;

  const auto& children = child_c->children;
  for (const auto child_e : children) {
    const auto* wep_c = r.try_get<WeaponComponent>(child_e);
    if (!wep_c)
      continue;
    weapons.push_back(child_e);
  }

  return weapons;
}

} // namespace game2d