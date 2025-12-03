#include "pch.hpp"

#include "ui_survive_onboarding_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "steam/isteaminput.h"

namespace game2d {

std::string
get_confirm_button_str(entt::registry& r, InputHandle_t handle)
{
  const auto& steam_c = get_first_component<SINGLE_SteamMappings>(r);

  std::string confirm_str = "...";

  if (handle != 0)
    confirm_str = get_str_for_da(steam_c, handle, DigitalAction::Game_South);

  else
    confirm_str = "E";

  return confirm_str;
}

std::string
get_back_button_str(entt::registry& r, InputHandle_t handle)
{
  const auto& steam_c = get_first_component<SINGLE_SteamMappings>(r);

  std::string back_str = "...";

  if (handle != 0)
    back_str = get_str_for_da(steam_c, handle, DigitalAction::Game_East);

  else
    back_str = "Q";

  return back_str;
}

} // namespace game2d