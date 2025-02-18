#pragma once

#include "modules/system_upgrade/upgrade_components.hpp"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct WeaponData
{
  std::string name;
  std::unordered_map<std::string, float> data;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WeaponData, name, data);
};

struct SINGLE_Weapons
{
  std::vector<WeaponData> weapons;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Weapons, weapons);
};

SINGLE_Weapons
load_weapons(std::string directory);

} // namespace game2d