#pragma once

#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct UpgradeEvent
{
  entt::entity par_e = entt::null;
  std::vector<entt::entity> upg_es;

  UpgradeRollResult roll_result; // rarity, stat, or trait
};

} // namespace game2d