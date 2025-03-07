#pragma once

#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_common/ui_common_components.hpp"

namespace game2d {

struct SINGLE_LevelUpUI
{
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
};

struct UpgradeResultsComponent
{
  std::vector<UpgradeRollResult> results;
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
    UpgradeableStat::ACTOR_STAMINA,
    UpgradeableStat::ACTOR_XP_ZONE_SIZE,
  
    // UpgradeableStat::BULLET_BOUNCE,
    UpgradeableStat::BULLET_CRIT_CHANCE,
    UpgradeableStat::BULLET_CRIT_DAMAGE,
    UpgradeableStat::BULLET_DAMAGE,
    UpgradeableStat::BULLET_KNOCKBACK,
    UpgradeableStat::BULLET_LIFESTEAL,   // %hp you recover when a bullet hits
    UpgradeableStat::BULLET_PIERCE,
    UpgradeableStat::BULLET_SIZE,
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