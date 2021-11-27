#pragma once

// other library
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

// c++ standard lib headers
#include <map>
#include <vector>

namespace engine {

// This class gets keyboard input, and gets controller input
class InputManager
{
public:
  InputManager();
  void new_frame();
  void process_key_down(const SDL_Scancode button, const Uint8 is_repeat);
  void process_key_up(const SDL_Scancode button, const Uint8 is_repeat);
  void process_mouse_event(const SDL_MouseButtonEvent& mouse_e);

  [[nodiscard]] bool get_key_down(SDL_Scancode button);
  [[nodiscard]] bool get_key_up(SDL_Scancode button);
  [[nodiscard]] bool get_key_held(SDL_Scancode button) const;

  // mouse
  [[nodiscard]] bool get_mouse_lmb_held() const;
  [[nodiscard]] bool get_mouse_rmb_held() const;
  [[nodiscard]] bool get_mouse_mmb_held() const;
  [[nodiscard]] bool get_mouse_lmb_down() const;
  [[nodiscard]] bool get_mouse_rmb_down() const;
  [[nodiscard]] bool get_mouse_mmb_down() const;
  [[nodiscard]] glm::ivec2 get_mouse_pos() const;
  void set_mousewheel_y(const float amount);
  [[nodiscard]] float get_mousewheel_y() const;

  // controller
  void open_controllers();

  [[nodiscard]] bool get_button_down(SDL_GameController* controller, SDL_GameControllerButton button);
  [[nodiscard]] bool get_button_up(SDL_GameController* controller, SDL_GameControllerButton button);
  [[nodiscard]] bool get_button_held(SDL_GameController* controller, SDL_GameControllerButton button);
  // [[nodiscard]] Sint16 get_axis_raw(SDL_GameController* controller, SDL_GameControllerAxis axis);
  // [[nodiscard]] float get_axis_dir(SDL_GameController* controller, SDL_GameControllerAxis axis);
  // [[nodiscard]] bool get_axis_held(SDL_GameController* controller, SDL_GameControllerAxis axis);

private:
  // keyboard state
  const Uint8* state;
  std::vector<SDL_Scancode> keys_pressed;
  std::vector<SDL_Scancode> keys_released;

  // mouse state
  float mousewheel_y;
  bool mouse_rmb_down = false;
  bool mouse_lmb_down = false;
  bool mouse_mmb_down = false;

public:
  // controller state
  // const int JOYSTICK_DEAD_ZONE = 8000;
  std::map<int, SDL_GameController*> controllers;
};

} // namespace engien
