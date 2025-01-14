#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct MenuBar
{
  bool enabled = true;
  std::string name;
};

struct SINGLE_DebugMenuBar
{
  std::vector<MenuBar> windows;
};

} // namespace game2d