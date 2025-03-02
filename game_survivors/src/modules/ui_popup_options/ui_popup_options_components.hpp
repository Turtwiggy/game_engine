#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct RequestToShowOptionsMenu
{
  bool placeholder = true;
};

struct SINGLE_OptionsMenuState
{
  bool open = false;
};

} // namespace game2d