#pragma once

#include "modules/core/ui/ui_common_components.hpp"

namespace game2d {

struct SINGLE_InfoUI : public DefaultUI
{
  // with the onboarding ui, track the readay state percent (0, 1)
  std::vector<float> player_ready_percents;

  const float time_to_confirm_max = 0.5f;

  bool complete = false;
};

} // namespace game2d