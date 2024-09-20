#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

struct SINGLE_SelectedUI
{
  std::vector<entt::entity> instantiated_ui;
};

struct SelectedUIComponent
{
  bool placeholder = true;
};

} // namespace game2d