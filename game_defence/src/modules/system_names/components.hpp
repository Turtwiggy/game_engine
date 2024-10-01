#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_NamesComponent
{
  std::vector<std::string> name;
};

struct NameComponent
{
  std::string full_name;
  std::string first_name;
  std::string last_name;
};

} // namespace game2d