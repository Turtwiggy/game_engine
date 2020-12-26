
// header
#include "engine/core/input_manager.hpp"

// c++ standard lib headers
#include <vector>

namespace fightingengine {

InputManager::InputManager()
{
  down = std::vector<SDL_Keycode>();
  state = SDL_GetKeyboardState(NULL);
}

void
InputManager::new_frame()
{
  down.clear();
}

// keyboard

void
InputManager::add_button_down(SDL_Keycode button)
{
  down.push_back(button);
}

bool
InputManager::get_key_down(SDL_Keycode button)
{
  return std::find(down.begin(), down.end(), button) != down.end();
}

bool
InputManager::get_key_held(SDL_Scancode button)
{
  return state[button];
}

// mouse

void
InputManager::add_mouse_down(SDL_MouseButtonEvent& mouse_e)
{
  if (mouse_e.button == SDL_BUTTON_LEFT) {
    printf("left mouse clicked\n");
  }
  if (mouse_e.button == SDL_BUTTON_RIGHT) {
    printf("right mouse clicked\n");
  }
  if (mouse_e.button == SDL_BUTTON_MIDDLE) {
    printf("middle mouse clicked\n");
  }
}

bool
InputManager::get_mouse_lmb_held()
{
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
}

bool
InputManager::get_mouse_rmb_held()
{
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
}

bool
InputManager::get_mouse_mmb_held()
{
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);
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

} // namespace fightingengine
