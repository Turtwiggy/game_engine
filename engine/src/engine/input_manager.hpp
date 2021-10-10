#pragma once

// other library
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

// c++ standard lib headers
#include <vector>

// TODO: delete class and put in to event key buffer

namespace engine {

class InputManager
{
public:
  InputManager();

  void new_frame();

  // keyboard

  const Uint8* get_keyboard_state() const;

  // return true when the button is pressed for the first time, until released
  [[nodiscard]] bool get_key_down(SDL_Scancode button);
  // return true when the button is released for the first itme after press
  [[nodiscard]] bool get_key_up(SDL_Scancode button);
  [[nodiscard]] bool get_key_held(SDL_Scancode button) const;

  // mouse
  [[nodiscard]] glm::ivec2 get_mouse_pos() const;

  void add_mouse_down(const SDL_MouseButtonEvent& mouse_e);
  [[nodiscard]] bool get_mouse_lmb_held() const;
  [[nodiscard]] bool get_mouse_rmb_held() const;
  [[nodiscard]] bool get_mouse_mmb_held() const;

  [[nodiscard]] bool get_mouse_lmb_down() const;
  [[nodiscard]] bool get_mouse_rmb_down() const;
  [[nodiscard]] bool get_mouse_mmb_down() const;

  void set_mousewheel_y(const float amount);
  [[nodiscard]] float get_mousewheel_y() const;

  // controller

  const int JOYSTICK_DEAD_ZONE = 8000;

  [[nodiscard]] Sint16 get_axis_raw(SDL_GameController* controller, SDL_GameControllerAxis axis);
  [[nodiscard]] float get_axis_dir(SDL_GameController* controller, SDL_GameControllerAxis axis);
  //[[nodiscard]] bool get_button_down(SDL_GameController* controller, SDL_GameControllerButton button);
  //[[nodiscard]] bool get_button_up(SDL_GameController* controller, SDL_GameControllerButton button);
  [[nodiscard]] bool get_button_held(SDL_GameController* controller, SDL_GameControllerButton button);
  [[nodiscard]] bool get_axis_held(SDL_GameController* controller, SDL_GameControllerAxis axis);

private:
  //
  // Keyboard state
  //
  const Uint8* state;

  // this is so we can keep a tally of kd, or ku over time
  std::vector<SDL_Scancode> kd;
  std::vector<SDL_Scancode> ku;

  // this is so we can keep knowledge of kd or ku
  // for all functions that call get ku or kd in a frame
  std::vector<SDL_Scancode> kd_cache;
  std::vector<SDL_Scancode> ku_cache;

  //
  // Mouse state
  //
  float mousewheel_y;

  bool mouse_rmb_down = false;
  bool mouse_lmb_down = false;
  bool mouse_mmb_down = false;
};

}
