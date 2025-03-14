#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class WEAPON_TYPE
{
  PROJECTILE,
  DEPLOY,
};

struct WeaponData
{
  std::string key;
  std::string name;
  std::string desc;
  std::string weapon_type;

  // validate on load
  WEAPON_TYPE type_as_enum = WEAPON_TYPE::PROJECTILE;

  std::unordered_map<std::string, float> data;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WeaponData, key, name, desc, weapon_type, data);
};

struct SINGLE_Weapons
{
  std::vector<WeaponData> weapons;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Weapons, weapons);
};

} // namespace game2d