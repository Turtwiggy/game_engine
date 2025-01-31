#pragma once

#include "modules/system_upgrade/upgrade_components.hpp"
#include <entt/entt.hpp>

namespace game2d {

struct UpgradeEvent
{
  entt::entity e = entt::null;
  Upgrade upgrade;
};

} // namespace game2d