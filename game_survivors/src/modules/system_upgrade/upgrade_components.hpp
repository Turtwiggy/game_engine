#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace game2d {

enum class UpgradeableStat
{
  BULLET_SPEED = 0,
  BULLET_DAMAGE,
  BULLET_PIERCE,
  BULLET_KNOCKBACK,

  WEAPON_FIRERATE,
  WEAPON_PROJECTILES,
  WEAPON_SPREAD,

  ACTOR_MAX_HEALTH,

  // if you have the missile trait,
  // peridically summon in missiles.
  // MISSILE_RADIUS,
  // MISSILE_DAMAGE,

  count,
};

// Loaded Data

struct Effects
{
  std::string type;
  std::map<std::string, nlohmann::json> params;
};

struct Upgrade
{
  std::string name;
  std::string desc;
  std::vector<Effects> effects;
};

struct SINGLE_Upgrades
{
  std::vector<Upgrade> upgrades;
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

      // This approach stacks the modifiers
      // result = modifier->apply(result);

      // This approach adds increases to the base
      float modified = modifier->apply(base);
      result += modified - base;
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

  float apply(float base) const override { return base * (1.0f + percent / 100.0f); }
};

struct StatFlatIncrease : public IStatModifier
{
private:
  float increase = 0.0f;

public:
  StatFlatIncrease(float increase, const std::string& stat)
    : IStatModifier{ "stat_flat_increase", stat }
    , increase(increase) {};

  float apply(float base) const override { return base + increase; }
};

} // namespace game2d
