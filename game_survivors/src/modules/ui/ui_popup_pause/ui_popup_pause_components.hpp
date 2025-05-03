#pragma once

#include "modules/core/ui/ui_common_components.hpp"

namespace game2d {

struct RequestToShowPauseMenu
{
  bool placeholder = true;
};

struct SINGLE_PauseMenuState : public DefaultUI
{
  void do_init(entt::registry& r) override;
};

} // namespace game2d