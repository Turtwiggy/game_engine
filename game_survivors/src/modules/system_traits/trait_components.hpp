#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class AquirableTrait
{
  ASSASSIN = 0,
  SPLINTER,
  FAN_FIRE,

  EXPLODE,    // on death
  DIRECT,     // ai type
  PROJECTILE, // ai type

  count,
};

struct TraitOnDisk
{
  std::string key;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TraitOnDisk, key);
};

struct TraitComponent
{
  std::unordered_set<AquirableTrait> traits;
};

} // namespace game2d