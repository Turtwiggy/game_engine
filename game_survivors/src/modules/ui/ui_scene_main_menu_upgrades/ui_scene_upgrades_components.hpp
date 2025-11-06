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
  // hold a button to purchase upgrade
  float purchase_time = 0.0f;
  const float purchase_time_max = 1.0f;

  void do_init(entt::registry& r) override;
};

} // namespace game2d