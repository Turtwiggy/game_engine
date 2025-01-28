#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class AquirableTrait
{
  ASSASSIN = 0,
  // BOUNCY_BULLET, // TODO

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
  std::vector<AquirableTrait> traits;
};

} // namespace game2d