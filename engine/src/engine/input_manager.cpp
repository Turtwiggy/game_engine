
// header
#include "engine/input_manager.hpp"

// your proj headers
#include "engine/maths_core.hpp"

// c++ standard lib headers
#include <imgui.h>

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

  kd_cache.clear();
  ku_cache.clear();
}

// keyboard

const Uint8*
InputManager::get_keyboard_state() const
{
  return state;
}

bool
InputManager::get_key_down(SDL_Scancode button)
{
  // check cache for this frame
  auto& cache_button_down = std::find(kd_cache.begin(), kd_cache.end(), button);
  if (cache_button_down != kd_cache.end())
    return true;

  bool button_held = get_key_held(button);
  auto& button_down = std::find(kd.begin(), kd.end(), button);

  // button held, and no recollection of it being pressed
  if (button_held && button_down == kd.end()) {
    kd.push_back(button);
    kd_cache.push_back(button);
    return true;
  }

  // button released, but we have it in the list of keys
  if (!button_held && button_down != kd.end()) {
    kd.erase(button_down);
  }

  return false;
}

bool
InputManager::get_key_up(SDL_Scancode button)
{
  // check cache for this frame
  auto& cache_button_up = std::find(ku_cache.begin(), ku_cache.end(), button);
  if (cache_button_up != ku_cache.end())
    return true;

  bool button_held = get_key_held(button);
  auto& button_up = std::find(ku.begin(), ku.end(), button);

  // button held, no recollection of it being pressed
  if (!button_held && button_up != ku.end()) {
    ku_cache.push_back(button);
    ku.erase(button_up);
    return true;
  }

  // button is pressed
  if (button_held && button_up == ku.end())
    ku.push_back(button);

  return false;
}

bool
InputManager::get_key_held(SDL_Scancode button) const
{
  if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse) {
    return false; // Imgui stole the event
  }
  return state[button];
}

// mouse

void
InputManager::add_mouse_down(const SDL_MouseButtonEvent& mouse_e)
{
  if (mouse_e.button == SDL_BUTTON_LEFT) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    // printf("(input_manager) left mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
    mouse_lmb_down = true;
  }
  if (mouse_e.button == SDL_BUTTON_RIGHT) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    // printf("(input_manager) right mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
    mouse_rmb_down = true;
  }
  if (mouse_e.button == SDL_BUTTON_MIDDLE) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    // printf("(input_manager) middle mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
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

glm::ivec2
InputManager::get_mouse_pos() const
{
  int x, y = 0;
  SDL_GetMouseState(&x, &y);
  return glm::ivec2(x, y);
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

Sint16
InputManager::get_axis_raw(SDL_GameController* controller, SDL_GameControllerAxis axis)
{
  Sint16 val = SDL_GameControllerGetAxis(controller, axis);
  return val;
}

float
InputManager::get_axis_dir(SDL_GameController* controller, SDL_GameControllerAxis axis)
{
  Sint16 val = SDL_GameControllerGetAxis(controller, axis);

  // add deadzone
  if (val < 0.0f && val > -JOYSTICK_DEAD_ZONE)
    return 0.0f;
  if (val >= 0.0f && val < JOYSTICK_DEAD_ZONE)
    return 0.0f;

  return scale(val, -32768.0f, 32767.0f, -1.0f, 1.0f);
}
bool
InputManager::get_axis_held(SDL_GameController* controller, SDL_GameControllerAxis axis)
{
  return get_axis_dir(controller, axis) > 0.0f;
}

// bool
// InputManager::get_button_down(SDL_GameController* controller, SDL_GameControllerButton button)
// {
//   // todo
//   return false;
// }
// bool
// InputManager::get_button_up(SDL_GameController* controller, SDL_GameControllerButton button)
// {
//   // todo
//   return true;
// }

bool
InputManager::get_button_held(SDL_GameController* controller, SDL_GameControllerButton button)
{
  Uint8 val = SDL_GameControllerGetButton(controller, button);
  return val != 0;
}

} // namespace engine
