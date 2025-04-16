#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class AquirableTrait
{
  EXPLODE,       // on death
  DIRECT,        // ai type
  ARC_ANGLE,     // ai type
  KEEP_DISTANCE, // ai type
  PROJECTILE,    // ai type
  CHARGE,        // ai type

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