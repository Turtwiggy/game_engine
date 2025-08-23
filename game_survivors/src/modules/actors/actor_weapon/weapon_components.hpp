#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class WEAPON_DAMAGE
{
  KINETIC,
  FIRE,
  ICE,
  SHOCK,
  POISON
};

enum class WEAPON_TYPE
{
  PROJECTILE,
  DEPLOY, // e.g. drop turrets, grenades
  AREA,   // e.g. flamethrower

  // DRONE,
};

struct Weapon_OnDiskData
{
  std::string key;
  std::string name;
  std::string desc;
  std::vector<std::string> audio;
  std::string weapon_type;
  std::string weapon_damage;

  // validate on load
  WEAPON_TYPE type_as_enum = WEAPON_TYPE::PROJECTILE;
  WEAPON_DAMAGE damage_as_enum = WEAPON_DAMAGE::KINETIC;

  std::unordered_map<std::string, float> data;
  std::vector<std::string> upgrades; // assigned upgrades (keys; data stored in WeponUpgrade_OnDiskData)

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Weapon_OnDiskData, key, name, desc, audio, weapon_type, weapon_damage, data, upgrades);
};

struct WeaponLevelComponent
{
  int level = 1;
};

struct WeaponDamageTypeComponent
{
  WEAPON_DAMAGE type = WEAPON_DAMAGE::KINETIC;
};

struct Stat
{
  std::string stat; // validate as valid UpgradeableStat
  std::string type; // flat or percent
  float value;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Stat, stat, type, value);

  // spaceship operator
  auto operator<=>(const Stat&) const = default;
};

struct WeaponUpgrade_OnDiskData
{
  std::string u_key;  // key unique to the upgrade
  std::string wb_key; // WeaponBehaviour key
  std::string desc;
  std::vector<Stat> stats;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponUpgrade_OnDiskData, u_key, wb_key, desc, stats);
};

struct SINGLE_Weapons
{
  std::vector<WeaponUpgrade_OnDiskData> weapon_upgrades;
  std::vector<Weapon_OnDiskData> weapons;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Weapons, weapons, weapon_upgrades);
};

} // namespace game2d