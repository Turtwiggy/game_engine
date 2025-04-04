#pragma once

#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_common/ui_common_components.hpp"

#include <unordered_map>
#include <unordered_set>

namespace game2d {

struct SINGLE_LevelUpUI
{
  bool open = false;

  std::vector<UIState> ui_states;
};

enum class Rarity
{
  COMMON = 1,
  UNCOMMON,
  RARE,
  LEGENDARY,
  SUPER_LEGENDARY,

  count
};

struct UpgradeRollResult
{
  Rarity rarity = Rarity::COMMON;
  UpgradeableStat upgrade;

  // Define the == operator as a member function
  bool operator==(const UpgradeRollResult& other) const { return rarity == other.rarity && upgrade == other.upgrade; }
};

struct UpgradeRollResult_hash
{
  std::size_t operator()(const UpgradeRollResult& key) const
  {
    const auto hash1 = std::hash<int>{}((int)key.rarity);
    const auto hash2 = std::hash<int>{}((int)key.upgrade);
    return hash1 ^ (hash2 << 1);
  }
};

struct UpgradeResultsComponent
{
  // note: this is a set so that all upgrades are unique.
  std::unordered_set<UpgradeRollResult, UpgradeRollResult_hash> results;
};

struct UpgradeNameOnDisk
{
  std::string stat;
  std::string rarity;
  std::string name;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(UpgradeNameOnDisk, stat, rarity, name);
};

struct SINGLE_UpgradeToName
{
  std::vector<UpgradeNameOnDisk> names;

  // to populate on load
  std::unordered_map<UpgradeRollResult, std::string, UpgradeRollResult_hash> stat_to_name_map;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_UpgradeToName, names);
};

constexpr std::array<std::pair<Rarity, int>, 5> rarity_chance_map = { {
  { Rarity::COMMON, 40 },
  { Rarity::UNCOMMON, 40 },
  { Rarity::RARE, 10 },
  { Rarity::LEGENDARY, 7 },
  { Rarity::SUPER_LEGENDARY, 3 },
} };

const std::vector<UpgradeableStat> traits_to_level_up = {
  // clang-format off
    UpgradeableStat::ACTOR_DODGE_CHANCE,
    UpgradeableStat::ACTOR_HEALTH_MAX,
    UpgradeableStat::ACTOR_HEALTH_REGEN,
    UpgradeableStat::ACTOR_SPEED,
    // UpgradeableStat::ACTOR_STAMINA,
    UpgradeableStat::ACTOR_XP_ZONE_SIZE,
  
    // UpgradeableStat::BULLET_BOUNCE,
    UpgradeableStat::BULLET_CRIT_CHANCE,
    UpgradeableStat::BULLET_CRIT_DAMAGE,
    UpgradeableStat::BULLET_DAMAGE,
    UpgradeableStat::BULLET_KNOCKBACK,
    UpgradeableStat::BULLET_LIFESTEAL,   // %hp you recover when a bullet hits
    UpgradeableStat::BULLET_PIERCE,
    // UpgradeableStat::BULLET_SIZE,
    UpgradeableStat::BULLET_SPEED,
  
    UpgradeableStat::WEAPON_CLIP_SIZE,
    UpgradeableStat::WEAPON_FIRERATE,
    // UpgradeableStat::WEAPON_PROJECTILES, // how many bullets to fire per shot
    // UpgradeableStat::WEAPON_SPREAD,      // at what angles
    UpgradeableStat::WEAPON_RELOAD,
    UpgradeableStat::WEAPON_RANGE
  // clang-format on
};

} // namespace game2d