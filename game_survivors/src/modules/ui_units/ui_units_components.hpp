#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct UnitOnDiskV0
{
  int version = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(UnitOnDiskV0, version);
};

struct UnitOnDiskV1 : public UnitOnDiskV0
{
  UnitOnDiskV1() { version = 1; };

  // new properties
  std::string name = "randomname";
  bool active = false;
  bool permadead = false;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UnitOnDiskV1, version, name, active, permadead);
};

struct UnitOnDiskV2 : public UnitOnDiskV1
{
  UnitOnDiskV2() { version = 2; };

  // new properties
  std::string another_property = "example";

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UnitOnDiskV2, version, name, active, permadead, another_property);
};

//
// what version to save units as?
//
using UnitType = UnitOnDiskV1;

//
// The component attached to the entity
// representing the loaded state from disk
//
struct UnitPersistentState
{
  bool active = false;
  bool permadead = false;
};

} // namespace game2d