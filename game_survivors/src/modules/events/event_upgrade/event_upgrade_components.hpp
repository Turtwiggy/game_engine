#pragma once

#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct UpgradeEvent
{
  entt::entity e = entt::null; // could be e.g. player_e

  UpgradeRollResult roll_result; // rarity, stat, or trait
};

} // namespace game2d