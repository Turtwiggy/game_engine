#pragma once

#include "modules/core/ui/ui_common_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct UI_BackButton : public DefaultUI
{
  void do_init(entt::registry& r) override;
};

} // namespace game2d