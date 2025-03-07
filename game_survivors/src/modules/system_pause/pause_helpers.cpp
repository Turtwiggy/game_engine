#include "pause_helpers.hpp"

#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"

namespace game2d {

bool
require_pause(entt::registry& r)
{
  bool pause = false;

  pause |= is_choosing_upgrade(r);

  return pause;
};

} // namespace game2d