#pragma once

#include "modules/core/ui/ui_common_components.hpp"

#include <imgui.h>

namespace game2d {

struct SINGLE_MainMenuUI : public DefaultUI
{
  int v_index = 0;
  // std::vector<std::string> hovered_buttons;

  void do_init(entt::registry& r) override;
};

struct RequestToShowMainMenu
{
  bool placeholder = true;
};

struct MenuToNextSceneInfo
{
  int level = 0;

  bool processed = false;
};

} // namespace game2d