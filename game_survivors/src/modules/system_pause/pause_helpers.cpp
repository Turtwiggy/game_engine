#include "pause_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/system_gameover/gameover_helpers.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui_popup_pause/ui_popup_pause_components.hpp"
#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"

namespace game2d {

bool
require_pause(entt::registry& r)
{
  bool pause = false;

  pause |= is_choosing_upgrade(r);
  pause |= is_gameover(r);

  // pause when pause menu is open
  const auto& pause_menu_c = get_first_component<SINGLE_PauseMenuState>(r);
  pause |= pause_menu_c.open;

  // pause when options menu is open
  const auto& options_menu_c = get_first_component<SINGLE_OptionsMenuState>(r);
  pause |= options_menu_c.open;

  return pause;
};

} // namespace game2d