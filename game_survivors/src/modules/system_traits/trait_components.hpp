#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct Trait
{
  std::string key;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Trait, key);
};

struct TraitComponent
{
  std::vector<Trait> traits;
};

} // namespace game2d