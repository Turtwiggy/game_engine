#pragma once

#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct StatUpgrade
{
  std::string type; // flat or percent
  Rarity rarity;
  UpgradeableStat stat;
  float value = 0.0f;
};

struct UpgradeEvent
{
  entt::entity e = entt::null; // could be e.g. player_e

  StatUpgrade data;
};

} // namespace game2d