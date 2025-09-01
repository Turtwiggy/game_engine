#pragma once

#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_components.hpp"

#include <unordered_map>
#include <unordered_set>

namespace game2d {

struct SINGLE_LevelUpUI
{
  bool open = false;

  std::vector<UIState> ui_states;
  std::vector<UiCursorComponent> ui_cursors;
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

// https://stackoverflow.com/a/72073933/1609322
inline std::size_t
hash_vector(const std::vector<uint32_t>& vec)
{
  std::size_t seed = vec.size();
  for (auto x : vec) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
};

struct UpgradeRollResult
{
  Rarity rarity = Rarity::COMMON;

  // When you upgrade, you can upgrade a stat,
  // or some custom gameplay behaviour.
  std::vector<Stat> stats;
  std::vector<WeaponBehaviour> traits;
  std::vector<entt::entity> weapons;
  bool level_weapons = false;

  // spaceship operator
  auto operator<=>(const UpgradeRollResult&) const = default;
};

struct UpgradeRollResult_hash
{
  std::size_t operator()(const UpgradeRollResult& key) const
  {
    std::vector<uint32_t> stats_as_ints;
    std::vector<uint32_t> traits_as_ints;

    std::transform(key.stats.begin(), key.stats.end(), std::back_inserter(stats_as_ints), [](const Stat s) {
      // convert stat to hash
      const auto hash1 = (uint32_t)std::hash<std::string>{}(s.stat);
      const auto hash2 = (uint32_t)std::hash<std::string>{}(s.type);
      const auto hash3 = (uint32_t)std::hash<float>{}(s.value);
      return hash_vector(std::vector<uint32_t>{ hash1, hash2, hash3 });
    });

    std::transform(key.traits.begin(), key.traits.end(), std::back_inserter(traits_as_ints), [](const WeaponBehaviour w) {
      return (uint32_t)w;
    });

    const uint32_t hash1 = (uint32_t)std::hash<int>{}((int)key.rarity);
    const uint32_t hash2 = (uint32_t)hash_vector(stats_as_ints);
    const uint32_t hash3 = (uint32_t)hash_vector(traits_as_ints);
    return hash_vector(std::vector<uint32_t>{ hash1, hash2, hash3 });
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

struct UpgradeValue
{
  std::string rarity;
  float value;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(UpgradeValue, rarity, value);
};

struct UpgradeValueOnDisk
{
  std::string stat; // e.g. ACTOR_DODGE_CHANCE
  std::string type; // stat_flat_increase or stat_percent_increase
  std::vector<UpgradeValue> values;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(UpgradeValueOnDisk, stat, type, values);
};

struct SINGLE_UpgradeToValue
{
  std::vector<UpgradeValueOnDisk> upgrades;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_UpgradeToValue, upgrades);
};

const std::vector<UpgradeableStat> upgradeable_weapon_stats{
  UpgradeableStat::WEAPON_CLIP_SIZE,
  UpgradeableStat::WEAPON_FIRERATE,
  UpgradeableStat::WEAPON_RELOAD,
  UpgradeableStat::WEAPON_RANGE,
};

const std::vector<UpgradeableStat> upgradeable_bullet_stats{
  // clang-format off

  UpgradeableStat::BULLET_CRIT_CHANCE,
  UpgradeableStat::BULLET_CRIT_DAMAGE,
  UpgradeableStat::BULLET_DAMAGE,
  UpgradeableStat::BULLET_KNOCKBACK,
  UpgradeableStat::BULLET_LIFESTEAL, // %hp you recover when a bullet hits
  UpgradeableStat::BULLET_PIERCE,
  // UpgradeableStat::BULLET_SIZE,
  // UpgradeableStat::BULLET_SPEED,

  // clang-format on
};

const std::vector<UpgradeableStat> upgradeable_area_stats{
  // clang-format off

  // UpgradeableStat::AREA_BEAMS_PER_WEAPON,
  // UpgradeableStat::AREA_STACKS_PER_SHOT, // e.g. stacks = 1stack * modifier
  UpgradeableStat::AREA_STACK_DAMAGE,         // e.g. damage = 3stack * dmg * modifier
  UpgradeableStat::AREA_STACK_DURATION,       // seconds to reduce one stack (lifetime)
  // UpgradeableStat::AREA_SIZE,                    // size of the area

  // clang-format on
};

const std::vector<UpgradeableStat> actor_x_stats{
  // clang-format off

  UpgradeableStat::ACTOR_DODGE_CHANCE,
  UpgradeableStat::ACTOR_HEALTH_MAX,
  UpgradeableStat::ACTOR_HEALTH_REGEN,
  UpgradeableStat::ACTOR_SPEED,
  // UpgradeableStat::ACTOR_STAMINA,
  UpgradeableStat::ACTOR_XP_ZONE_SIZE,

  // clang-format on
};

constexpr std::array<std::pair<Rarity, int>, 5> rarity_chance_map = { {
  { Rarity::COMMON, 40 },
  { Rarity::UNCOMMON, 40 },
  { Rarity::RARE, 10 },
  { Rarity::LEGENDARY, 7 },
  { Rarity::SUPER_LEGENDARY, 3 },
} };

std::pair<float, std::string>
get_stat_from_stat_table(entt::registry& r, Rarity rarity, UpgradeableStat upgrade);

const auto rarity_to_col = [](Rarity rarity) -> ImVec4 {
  if (rarity == Rarity::COMMON) {
    const auto srgb = hex_to_srgb("#D9D9D9"); //  white
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::UNCOMMON) {
    const auto srgb = hex_to_srgb("#00c420"); //  green
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::RARE) {
    const auto srgb = hex_to_srgb("#00b6ff"); //  bright blue
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::LEGENDARY) {
    const auto srgb = hex_to_srgb("#cfc041"); //  gold
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::SUPER_LEGENDARY) {
    const auto srgb = hex_to_srgb("#d74200"); //  red
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  return { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct CardDataUI
{
  Rarity rarity = Rarity::COMMON;
  std::string rarity_txt = "common";
  std::string header_txt = "Bronze Hulls";
  std::string desc_txt = "+15 firerate";
  bool selected = false;
};

struct CardUIUpgradeComponent
{
  const float time_to_confirm_max = 0.2f;
  float time_to_confirm_cur = 0.0f;

  bool released_since_action = true;
};

} // namespace game2d