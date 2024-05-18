#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct PatrolDescription
{
  int speed_min = 20;
  int speed_max = 50;
  int units_min = 1;
  int units_max = 100;

  PatrolDescription() = default;
};

struct PatrolComponent
{
  int strength = 1;
  // bool should_patrol = true; // todo: implement
};
inline void
to_json(nlohmann::json& j, const PatrolComponent& c)
{
  j = nlohmann::json{ { "strength", static_cast<int>(c.strength) } };
};
inline void
from_json(const nlohmann::json& j, PatrolComponent& c)
{
  j.at("strength").get_to(c.strength);
};

struct BackstabbableComponent
{
  bool placeholder = true;
};

} // namespace game2d