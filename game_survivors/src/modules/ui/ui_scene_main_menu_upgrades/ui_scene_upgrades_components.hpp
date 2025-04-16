#pragma once

#include "modules/core/ui/ui_common_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"

#include <optional>

namespace game2d {

struct RequestToShowUpgradesMenu
{
  bool placeholder = true;
};

struct SINGLE_PersistentUpgradesMenuUI
{
  // prevent immediately doing action when sent by other scene
  bool one_frame_buffer = true;
  bool display = false;
  bool init = false;

  int grid_idx = 0;
  int grid_x = 8;
  UIState state;

  std::optional<UpgradeableStat> selected_stat = std::nullopt;
};

} // namespace game2d