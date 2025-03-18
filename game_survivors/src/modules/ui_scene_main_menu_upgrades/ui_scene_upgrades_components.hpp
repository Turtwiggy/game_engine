#pragma once

#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_common/ui_common_components.hpp"

#include <optional>

namespace game2d {

struct RequestToShowUpgradesMenu
{
  bool placeholder = true;
};

struct SINGLE_UpgradesMenuUI
{
  // prevent immediately doing action when sent by other scene
  bool one_frame_buffer = true;
  bool display = false;
  bool init = false;

  // how many columns
  int ui_col_index = 0; // which column
  std::vector<UIState> state;

  std::optional<UpgradeableStat> selected_stat = std::nullopt;
};

} // namespace game2d