#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

#include <optional>
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

struct Effects
{
  std::optional<std::string> stat = std::nullopt;
  std::optional<std::string> type = std::nullopt;
  std::optional<nlohmann::json> value = std::nullopt; // number or string
  std::optional<std::string> trait = std::nullopt;
  // std::map<std::string, nlohmann::json> params;

  friend void to_json(nlohmann ::json& j, const Effects& val)
  {
    if (val.stat.has_value())
      j["stat"] = val.stat.value();
    if (val.type.has_value())
      j["type"] = val.type.value();
    if (val.value.has_value())
      j["value"] = val.value.value();
    if (val.trait.has_value())
      j["trait"] = val.trait.value();
  }
  friend void from_json(const nlohmann ::json& j, Effects& val)
  {
    if (j.contains("stat"))
      j.at("stat").get_to(val.stat.emplace());
    if (j.contains("type"))
      j.at("type").get_to(val.type.emplace());
    if (j.contains("value"))
      j.at("value").get_to(val.value.emplace());
    if (j.contains("trait"))
      j.at("trait").get_to(val.trait.emplace());
  };
};

struct Upgrade
{
  std::string name;
  std::vector<Effects> effects;

  // either a manual desc, or generate desc from the effects
  std::optional<std::string> desc = std::nullopt;

  friend void to_json(nlohmann ::json& j, const Upgrade& val)
  {
    j["name"] = val.name;
    j["effects"] = val.effects;
    if (val.desc.has_value())
      j["desc"] = val.desc.value();
  }
  friend void from_json(const nlohmann ::json& j, Upgrade& val)
  {
    j.at("name").get_to(val.name);
    j.at("effects").get_to(val.effects);
    if (j.contains("desc"))
      j.at("desc").get_to(val.desc.emplace());
  };
};

struct SINGLE_Upgrades
{
  std::vector<Upgrade> upgrades;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Upgrades, upgrades);
};

struct UpgradeComponent
{
  std::vector<std::string> aquired_upgrades;
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
