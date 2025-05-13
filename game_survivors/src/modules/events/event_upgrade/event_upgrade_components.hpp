#pragma once

#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct UpgradeEvent
{
  entt::entity par_e = entt::null;
  entt::entity upg_e = entt::null;

  UpgradeRollResult roll_result; // rarity, stat, or trait
};

} // namespace game2d