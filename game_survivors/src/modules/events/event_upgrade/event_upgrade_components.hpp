#pragma once

#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct UpgradeEvent
{
  entt::entity e = entt::null;
  Upgrade upgrade;
};

} // namespace game2d