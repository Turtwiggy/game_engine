#include "pause_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/systems/system_gameover/gameover_helpers.hpp"
#include "modules/ui/ui_popup_controller_disconnected/ui_popup_controller_disconnected_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_popup_pause/ui_popup_pause_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"

namespace game2d {

bool
require_pause(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // Don't pause in the "main" scene.
  // otherwise this would count as a "pause" when options is open
  const auto& scene_c = SINGLE_CurrentScene::instance;
  if (scene_c.s == Scene::menu)
    return false;

  bool pause = false;

  pause |= is_choosing_upgrade(r);
  pause |= is_gameover(r);

  const auto& handles_c = get_first_component<SINGLE_DisconnectedControllerUI>(r).handle_disconnected;
  const bool disconnected = !(handles_c.empty());
  pause |= disconnected;

  // pause when pause menu is open
  if (get_first<SINGLE_PauseMenuState>(r) != entt::null) {
    const auto& pause_menu_c = get_first_component<SINGLE_PauseMenuState>(r);
    pause |= pause_menu_c.open;
  }

  // pause when options menu is open
  if (get_first<SINGLE_OptionsMenuState>(r) != entt::null) {
    const auto& options_menu_c = get_first_component<SINGLE_OptionsMenuState>(r);
    pause |= options_menu_c.open;
  }

  return pause;
};

} // namespace game2d