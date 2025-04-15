#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

#include <string>

namespace game2d {
using namespace nlohmann;

enum class UpgradeableStat
{
  ACTOR_DODGE_CHANCE = 0,
  ACTOR_HEALTH_MAX,
  ACTOR_HEALTH_REGEN,
  ACTOR_SPEED,
  ACTOR_STAMINA, // seconds to sprint
  ACTOR_XP_ZONE_SIZE,

  BULLET_BOUNCE,
  BULLET_CRIT_CHANCE,
  BULLET_CRIT_DAMAGE,
  BULLET_DAMAGE,
  BULLET_KNOCKBACK,
  BULLET_LIFESTEAL, // %hp you recover when a bullet hits
  BULLET_PIERCE,
  BULLET_SIZE,
  BULLET_SPEED,

  WEAPON_PROJECTILES, // how many bullets to fire per shot
  WEAPON_SPREAD,      // at what angles
  WEAPON_FIRERATE,
  WEAPON_CLIP_SIZE,
  WEAPON_RELOAD,
  WEAPON_RANGE,

  count,
};

// Loaded Data

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

struct SINGLE_Upgrades
{
  std::vector<Upgrade> upgrades;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Upgrades, upgrades);
};

//
// Types of supported Effects
//

struct IStatModifier
{
  std::string modifier = "stat_percent_increase";
  std::string stat = "bullet_speed";

  IStatModifier() = delete;
  IStatModifier(const std::string& modifier, const std::string& stat)
    : modifier(modifier)
    , stat(stat) {};

  virtual float apply(float base) const = 0;
  virtual ~IStatModifier() = default;
};

//
// Attach this to entities that want Upgrades
// And add a modifier when the upgrade is aquired.
// For example, an upgrade could add a modifier,
// that gives a "stat_percent_increase" to "bullet_speed" by 30%.
//
struct StatModifierComponent
{
  std::vector<std::shared_ptr<IStatModifier>> modifiers;

public:
  void add(std::shared_ptr<IStatModifier> modifier) { modifiers.push_back(modifier); };

  float apply_modifiers(float base, std::string stat) const
  {
    float result = base;

    for (const auto& modifier : modifiers) {

      if (modifier->stat != stat)
        continue;

      // multiply the result
      if (modifier->modifier == "stat_percent_increase")
        result *= modifier->apply(result);

      // add the flat number
      if (modifier->modifier == "stat_flat_increase")
        result += modifier->apply(base);
    }

    return result;
  }
};

// Modifiers that Upgrades can apply

struct StatPercentIncrease : public IStatModifier
{
private:
  float percent = 0.0f;

public:
  StatPercentIncrease(float percent_between_0_and_100, const std::string& stat)
    : IStatModifier{ "stat_percent_increase", stat }
    , percent(percent_between_0_and_100) {};

  float apply(float base) const override { return (1.0f + (percent / 100.0f)); }
};

struct StatFlatIncrease : public IStatModifier
{
private:
  float increase = 0.0f;

public:
  StatFlatIncrease(float increase, const std::string& stat)
    : IStatModifier{ "stat_flat_increase", stat }
    , increase(increase) {};

  float apply(float base) const override { return increase; }
};

} // namespace game2d
