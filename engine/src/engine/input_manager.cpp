
// header
#include "engine/input_manager.hpp"

// your proj headers
#include "engine/maths.hpp"

// other lib headers
#include <imgui.h>

// c++ standard lib headers
#include <algorithm>
#include <cstdio>

namespace engine {

InputManager::InputManager()
{
  state = SDL_GetKeyboardState(NULL);
}

void
InputManager::new_frame()
{
  mouse_lmb_down = false;
  mouse_rmb_down = false;
  mouse_mmb_down = false;
  mousewheel_y = 0.0f;

  keys_pressed.clear();
  keys_released.clear();
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

void
InputManager::process_mouse_event(const SDL_MouseButtonEvent& mouse_e)
{
  if (mouse_e.button == SDL_BUTTON_LEFT) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    mouse_lmb_down = true;
  }
  if (mouse_e.button == SDL_BUTTON_RIGHT) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    mouse_rmb_down = true;
  }
  if (mouse_e.button == SDL_BUTTON_MIDDLE) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    mouse_mmb_down = true;
  }
}

bool
InputManager::get_mouse_lmb_held() const
{
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
}

bool
InputManager::get_mouse_rmb_held() const
{
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
}

bool
InputManager::get_mouse_mmb_held() const
{
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);
}

bool
InputManager::get_mouse_lmb_down() const
{
  return mouse_lmb_down;
}

bool
InputManager::get_mouse_rmb_down() const
{
  return mouse_rmb_down;
}

bool
InputManager::get_mouse_mmb_down() const
{
  return mouse_mmb_down;
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
InputManager::open_controllers()
{
  int controllers = SDL_NumJoysticks();
  printf("(InputManager) %i controllers available \n", controllers);

  for (int i = 0; i < controllers; ++i) {
    if (SDL_IsGameController(i)) {
      // Open available controllers
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller) {
        this->controllers.push_back(controller);
        printf("(InputManager) controller loaded... %i \n", i);
      } else {
        fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
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
    controller_buttons_pressed[joystick_id] = std::vector<Uint8>();

  auto& buttons = controller_buttons_pressed[joystick_id];
  auto& button_location = std::find(buttons.begin(), buttons.end(), button);
  if (held && button_location == buttons.end()) {
    buttons.push_back(button);
    return true;
  }
  if (!held && button_location != buttons.end())
    buttons.erase(button_location);
  return false;
}

bool
InputManager::get_button_up(SDL_GameController* controller, SDL_GameControllerButton button)
{
  // not ideal, but the joy_event buttons dont seem
  // to align with SDL_GameControllerButton enum
  bool held = get_button_held(controller, button);

  SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
  SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);
  bool has_joystick = controller_buttons_pressed.find(joystick_id) != controller_buttons_pressed.end();
  if (!has_joystick)
    controller_buttons_pressed[joystick_id] = std::vector<Uint8>();

  auto& buttons = controller_buttons_pressed[joystick_id];
  auto& button_location = std::find(buttons.begin(), buttons.end(), button);
  if (!held && button_location != buttons.end()) {
    buttons.erase(button_location);
    return true;
  }
  if (held && button_location == buttons.end())
    buttons.push_back(button);
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
