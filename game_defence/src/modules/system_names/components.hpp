#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_NamesComponent
{
  std::vector<std::string> name;
};

struct NameComponent
{
  std::string name;
};

} // namespace game2d