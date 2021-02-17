
// header
#include "engine/input_manager.hpp"

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

  mouse_lmb_down = false;
  mouse_rmb_down = false;
  mouse_mmb_down = false;
}

// keyboard

const Uint8*
InputManager::get_keyboard_state() const
{
  return state;
}

void
InputManager::add_button_down(SDL_Keycode button)
{
  down.push_back(button);
}

bool
InputManager::get_key_down(SDL_Keycode button) const
{
  return std::find(down.begin(), down.end(), button) != down.end();
}

bool
InputManager::get_key_held(SDL_Scancode button) const
{
  return state[button];
}

// mouse

void
InputManager::add_mouse_down(SDL_MouseButtonEvent& mouse_e)
{
  if (mouse_e.button == SDL_BUTTON_LEFT) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    printf("(input_manager) left mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
    mouse_lmb_down = true;
  }
  if (mouse_e.button == SDL_BUTTON_RIGHT) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    printf("(input_manager) right mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
    mouse_rmb_down = true;
  }
  if (mouse_e.button == SDL_BUTTON_MIDDLE) {
    glm::ivec2 mouse_pos = get_mouse_pos();
    printf("(input_manager) middle mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
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

glm::vec2
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

} // namespace fightingengine
