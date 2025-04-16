#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class AiBehaviour
{
  EXPLODE,       // on death
  DIRECT,        // ai type
  ARC_ANGLE,     // ai type
  KEEP_DISTANCE, // ai type
  PROJECTILE,    // ai type
  CHARGE,        // ai type

  count,
};

struct KeyValueOnDisk
{
  std::string key;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(KeyValueOnDisk, key);
};

struct AiBehavioursComponent
{
  std::unordered_set<AiBehaviour> traits;
};

} // namespace game2d