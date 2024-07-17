#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct PatrolDescription
{
  int speed_min = 25;
  int speed_max = 45;
  int units_min = 4;
  int units_max = 10;

  PatrolDescription() = default;
};

struct PatrolComponent
{
  int strength = 1;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PatrolComponent, strength);
};

struct BackstabbableComponent
{
  bool placeholder = true;
};

} // namespace game2d