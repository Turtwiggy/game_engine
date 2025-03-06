#include "pause_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/ui_scene_survive_debug_level_up/ui_survive_level_up_components.hpp"

namespace game2d {

bool
require_pause(entt::registry& r)
{
  bool pause = false;

  // pause due to needing level up
  auto lv_up_e = get_first<SINGLE_LevelUpUI>(r);
  if (lv_up_e != entt::null) {
    const auto& lv_up_c = r.get<SINGLE_LevelUpUI>(lv_up_e);
    pause |= lv_up_c.require_level_up;
  }

  return pause;
};

} // namespace game2d