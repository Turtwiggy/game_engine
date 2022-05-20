
// header
#include "engine/app/input.hpp"

// your proj headers
#include "engine/maths/maths.hpp"

// other lib headers
#include <imgui.h>

// c++ standard lib headers
#include <algorithm>
#include <iostream>

namespace engine {

InputManager::InputManager()
{
  state = SDL_GetKeyboardState(NULL);
}

void
InputManager::new_frame(const uint64_t& frame)
{
  mousewheel_y = 0.0f;

  keys_pressed.clear();
  keys_released.clear();

  this->frame = frame;
}

//
// keyboard
//

void
InputManager::process_key_down(const SDL_Scancode button, const Uint8 is_repeat)
{
  if (is_repeat == 1)
    return; // dont process held
  keys_pressed.push_back(button);
}

void
InputManager::process_key_up(const SDL_Scancode button, const Uint8 is_repeat)
{
  if (is_repeat == 1)
    return; // dont process held
  keys_released.push_back(button);
}

bool
InputManager::get_key_down(const SDL_Scancode button)
{
  const auto& button_pressed = std::find(keys_pressed.begin(), keys_pressed.end(), button);
  return button_pressed != keys_pressed.end();
}

bool
InputManager::get_key_up(const SDL_Scancode button)
{
  const auto& button_pressed = std::find(keys_released.begin(), keys_released.end(), button);
  return button_pressed != keys_released.end();
}

bool
InputManager::get_key_held(SDL_Scancode button) const
{
  return state[button];
}

//
// mouse
//

bool
InputManager::get_mouse_lmb_held() const
{
  // return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
  return ImGui::IsMouseDown(ImGuiMouseButton_Left);
}

bool
InputManager::get_mouse_rmb_held() const
{
  // return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
  return ImGui::IsMouseDown(ImGuiMouseButton_Right);
}

bool
InputManager::get_mouse_mmb_held() const
{
  // return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);
  return ImGui::IsMouseDown(ImGuiMouseButton_Middle);
}

bool
InputManager::get_mouse_lmb_press() const
{
  return ImGui::IsMouseClicked(ImGuiMouseButton_Left);
}

bool
InputManager::get_mouse_rmb_press() const
{
  return ImGui::IsMouseClicked(ImGuiMouseButton_Right);
}

bool
InputManager::get_mouse_mmb_press() const
{
  return ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
}

bool
InputManager::get_mouse_lmb_release()
{
  return ImGui::IsMouseReleased(ImGuiMouseButton_Left);
}

bool
InputManager::get_mouse_rmb_release()
{
  return ImGui::IsMouseReleased(ImGuiMouseButton_Right);
}

bool
InputManager::get_mouse_mmb_release()
{
  return ImGui::IsMouseReleased(ImGuiMouseButton_Middle);
}

// Returns mouse pos relative to tl of screen
glm::ivec2
InputManager::get_mouse_pos() const
{
  // int x, y = 0;
  // SDL_GetMouseState(&x, &y);
  // return glm::ivec2(x, y);
  const auto& pos = ImGui::GetIO().MousePos;
  return glm::ivec2(pos.x, pos.y);
}

void
InputManager::set_mousewheel_y(const float amount)
{
  mousewheel_y = amount;
}

float
InputManager::get_mousewheel_y() const
{
  return mousewheel_y;
}

//
// controller
//

void
InputManager::process_controller_added()
{
  std::cout << "controller added... processing" << std::endl;

  for (auto* controller : controllers) {
    if (controller)
      SDL_GameControllerClose(controller);
  };
  controllers.clear();

  open_controllers();
};

void
InputManager::process_controller_removed()
{
  std::cout << "controller removed... processing" << std::endl;

  for (auto* controller : controllers) {
    if (controller)
      SDL_GameControllerClose(controller);
  };
  controllers.clear();

  open_controllers();
};

void
InputManager::open_controllers()
{
  int controllers = SDL_NumJoysticks();
  std::cout << "(InputManager) controllers available: " << controllers << std::endl;

  for (int i = 0; i < controllers; ++i) {
    if (SDL_IsGameController(i)) {
      // Open available controllers
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller) {
        this->controllers.push_back(controller);
        std::cout << "(InputManager) controller loaded... " << i << std::endl;
      } else {
        std::cerr << "Could not open gamecontroller " << i << " " << SDL_GetError() << std::endl;
      }
    }
  }
};

bool
InputManager::get_button_down(SDL_GameController* controller, SDL_GameControllerButton button)
{
  // not ideal, but the joy_event buttons dont seem
  // to align with SDL_GameControllerButton enum
  bool held = get_button_held(controller, button);

  SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
  SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);
  bool has_joystick = controller_buttons_pressed.find(joystick_id) != controller_buttons_pressed.end();
  if (!has_joystick)
    controller_buttons_pressed.insert({ joystick_id, {} });

  auto& buttons = controller_buttons_pressed[joystick_id];
  auto button_location = std::find_if(
    buttons.begin(), buttons.end(), [&button](const std::pair<uint64_t, Uint8> obj) { return obj.second == button; });

  // Button was pressed this frame, no recollection
  if (held && button_location == buttons.end()) {
    buttons.push_back({ this->frame, button });
    return true;
  }

  // Button exists this frame
  if (button_location != buttons.end()) {
    // If its still the same frame, return true
    const auto& button_added_frame = (*button_location).first;
    if (button_added_frame == this->frame)
      return true;
    // If its a different frame, remove it
    if (!held)
      buttons.erase(button_location);
  }
  return false;
}

bool
InputManager::get_button_up(SDL_GameController* controller, SDL_GameControllerButton button)
{
  bool pressed = get_button_down(controller, button);

  SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
  SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);
  auto& buttons = controller_buttons_pressed[joystick_id];
  auto button_location = std::find_if(
    buttons.begin(), buttons.end(), [&button](const std::pair<uint64_t, Uint8> obj) { return obj.second == button; });
  if (!pressed && button_location != buttons.end()) {
    return true;
  }
  return false;
}

bool
InputManager::get_button_held(SDL_GameController* controller, SDL_GameControllerButton button)
{
  Uint8 val = SDL_GameControllerGetButton(controller, button);
  return val != 0;
}

float
InputManager::get_axis_dir(SDL_GameController* controller, SDL_GameControllerAxis axis)
{
  const int JOYSTICK_DEAD_ZONE = 8000;

  Sint16 val = SDL_GameControllerGetAxis(controller, axis);

  // add deadzone
  if (val < 0.0f && val > -JOYSTICK_DEAD_ZONE)
    return 0.0f;
  if (val >= 0.0f && val < JOYSTICK_DEAD_ZONE)
    return 0.0f;

  return scale(val, -32768.0f, 32767.0f, -1.0f, 1.0f);
}

// Sint16
// InputManager::get_axis_raw(SDL_GameController* controller, SDL_GameControllerAxis axis)
// {
//   Sint16 val = SDL_GameControllerGetAxis(controller, axis);
//   return val;
// }

} // namespace engine
