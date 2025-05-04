#include "pch.hpp"

#include "actor_player_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"

namespace game2d {

bool
has_action(const std::vector<ActionStateEnum>& action, const ActionStateEnum act)
{
  auto it = std::find(action.begin(), action.end(), act);
  return it != action.end();
};

InputComponent
generate_from_handle(entt::registry& r, const InputHandle_t handle)
{
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& steam_gs_c = get_first_component<SINGLE_SteamControllerGameState>(r);

  InputComponent i;

  const auto l_analog = controller_axis(r, handle, AA::LAnalogControls);
  const auto r_analog = controller_axis(r, handle, AA::RAnalogControls);
  i.lx += l_analog.x;
  i.ly += -l_analog.y; // flip y
  i.rx += r_analog.x;
  i.ry += -r_analog.y; // flip y

  const auto generate_actions = [&steam_c, &handle](std::vector<ActionStateEnum>& acts, const DA& da) {
    if (controller_button_down(steam_c, handle, da))
      acts.push_back(ActionStateEnum::DOWN);
    if (controller_button_held(steam_c, handle, da))
      acts.push_back(ActionStateEnum::HELD);
    if (controller_button_release(steam_c, handle, da))
      acts.push_back(ActionStateEnum::RELEASE);
  };
  generate_actions(i.pause, DA::Game_Start);
  generate_actions(i.ability1, DA::Game_LB);
  generate_actions(i.ability2, DA::Game_RB);
  generate_actions(i.dpad_u, DA::Game_Up);
  generate_actions(i.dpad_d, DA::Game_Down);
  generate_actions(i.dpad_l, DA::Game_Left);
  generate_actions(i.dpad_r, DA::Game_Right);
  generate_actions(i.button_n, DA::Game_North);
  generate_actions(i.button_s, DA::Game_South);
  generate_actions(i.button_e, DA::Game_East);
  generate_actions(i.button_w, DA::Game_West);
  return i;
}

} // namespace game2d