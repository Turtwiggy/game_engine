#include "pch.hpp"

#include "actor_player_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"

namespace game2d {

void
merge_inputs(InputComponent& i, const InputComponent& input)
{
  i.lx += input.lx;
  i.ly += input.ly;
  i.rx += input.rx;
  i.ry += input.ry;
  i.lx = glm::clamp(i.lx, -1.0f, 1.0f);
  i.ly = glm::clamp(i.ly, -1.0f, 1.0f);
  i.rx = glm::clamp(i.rx, -1.0f, 1.0f);
  i.ry = glm::clamp(i.ry, -1.0f, 1.0f);

  i.pause.insert(i.pause.end(), input.pause.begin(), input.pause.end());
  i.ability1.insert(i.ability1.end(), input.ability1.begin(), input.ability1.end());
  i.ability2.insert(i.ability2.end(), input.ability2.begin(), input.ability2.end());
  i.dpad_u.insert(i.dpad_u.end(), input.dpad_u.begin(), input.dpad_u.end());
  i.dpad_d.insert(i.dpad_d.end(), input.dpad_d.begin(), input.dpad_d.end());
  i.dpad_l.insert(i.dpad_l.end(), input.dpad_l.begin(), input.dpad_l.end());
  i.dpad_r.insert(i.dpad_r.end(), input.dpad_r.begin(), input.dpad_r.end());
  i.button_n.insert(i.button_n.end(), input.button_n.begin(), input.button_n.end());
  i.button_s.insert(i.button_s.end(), input.button_s.begin(), input.button_s.end());
  i.button_e.insert(i.button_e.end(), input.button_e.begin(), input.button_e.end());
  i.button_w.insert(i.button_w.end(), input.button_w.begin(), input.button_w.end());
}

InputComponent
generate_from_keyboard(entt::registry& r)
{
  const auto& sdl_input_c = get_first_component<SINGLE_InputComponent>(r);

  InputComponent i;

  i.ly += get_key_held(sdl_input_c, SDL_SCANCODE_W) ? -1.0f : 0.0f;
  i.ly += get_key_held(sdl_input_c, SDL_SCANCODE_S) ? 1.0f : 0.0f;
  i.lx += get_key_held(sdl_input_c, SDL_SCANCODE_A) ? -1.0f : 0.0f;
  i.lx += get_key_held(sdl_input_c, SDL_SCANCODE_D) ? 1.0f : 0.0f;

  if (get_mouse_lmb_press())
    i.ability1.push_back(ActionStateEnum::DOWN);
  if (get_mouse_lmb_held())
    i.ability1.push_back(ActionStateEnum::HELD);
  if (get_mouse_lmb_release())
    i.ability1.push_back(ActionStateEnum::RELEASE);
  if (get_mouse_rmb_press())
    i.ability2.push_back(ActionStateEnum::DOWN);
  if (get_mouse_rmb_held())
    i.ability2.push_back(ActionStateEnum::HELD);
  if (get_mouse_rmb_release())
    i.ability2.push_back(ActionStateEnum::RELEASE);

  auto generate_actions_from_keyboard = [&sdl_input_c](std::vector<ActionStateEnum>& acts, const SDL_Scancode key) {
    if (get_key_down(sdl_input_c, key))
      acts.push_back(ActionStateEnum::DOWN);
    if (get_key_held(sdl_input_c, key))
      acts.push_back(ActionStateEnum::HELD);
    if (get_key_up(sdl_input_c, key))
      acts.push_back(ActionStateEnum::RELEASE);
  };
  generate_actions_from_keyboard(i.pause, SDL_SCANCODE_ESCAPE);
  generate_actions_from_keyboard(i.dpad_u, SDL_SCANCODE_UP);
  generate_actions_from_keyboard(i.dpad_d, SDL_SCANCODE_DOWN);
  generate_actions_from_keyboard(i.dpad_l, SDL_SCANCODE_LEFT);
  generate_actions_from_keyboard(i.dpad_r, SDL_SCANCODE_RIGHT);
  generate_actions_from_keyboard(i.button_n, SDL_SCANCODE_KP_8);
  generate_actions_from_keyboard(i.button_s, SDL_SCANCODE_KP_2);
  generate_actions_from_keyboard(i.button_e, SDL_SCANCODE_KP_6);
  generate_actions_from_keyboard(i.button_w, SDL_SCANCODE_KP_4);
  generate_actions_from_keyboard(i.button_s, SDL_SCANCODE_KP_ENTER);
  generate_actions_from_keyboard(i.button_s, SDL_SCANCODE_RETURN);
  generate_actions_from_keyboard(i.button_e, SDL_SCANCODE_ESCAPE);

  return i;
}

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