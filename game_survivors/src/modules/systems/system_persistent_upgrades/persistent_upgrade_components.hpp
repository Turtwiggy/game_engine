#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct UpgradeLevel
{
  int cost;         // cost to upgrade
  std::string type; // stat_percent_increase or stat_flat_increase
  float value;      // the stat value to increase by

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UpgradeLevel, cost, type, value);
};

struct Upgrade
{
  std::string key;
  std::vector<UpgradeLevel> levels;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Upgrade, key, levels);
};

struct SINGLE_PersistentUpgrades
{
  std::vector<Upgrade> upgrades;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_PersistentUpgrades, upgrades);
};

} // namespace game2d