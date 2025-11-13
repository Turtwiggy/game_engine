#pragma once

#include "modules/core/ui/ui_common_components.hpp"

namespace game2d {

struct SINGLE_UIAreYouSure : public DefaultUI
{
  std::optional<std::function<void(bool)>> action = std::nullopt;

  static SINGLE_UIAreYouSure instance;
};

} // namespace game2d