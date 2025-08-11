#pragma once

#include "modules/core/ui/ui_common_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"

#include <optional>

namespace game2d {

struct RequestToShowUpgradesMenu
{
  bool placeholder = true;
};

struct SINGLE_PersistentUpgradesMenuUI : public DefaultUI
{
  int grid_idx = 0;
  const int grid_x = 8;
  std::optional<UpgradeableStat> selected_stat = std::nullopt;

  void do_init(entt::registry& r) override;
};

} // namespace game2d