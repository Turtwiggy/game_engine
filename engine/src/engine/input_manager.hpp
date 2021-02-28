#pragma once

// c++ standard lib headers
#include <vector>

// other library
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

namespace fightingengine {

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
  [[nodiscard]] glm::vec2 get_mouse_pos() const;

  void add_mouse_down(SDL_MouseButtonEvent& mouse_e);
  [[nodiscard]] bool get_mouse_lmb_held() const;
  [[nodiscard]] bool get_mouse_rmb_held() const;
  [[nodiscard]] bool get_mouse_mmb_held() const;

  [[nodiscard]] bool get_mouse_lmb_down() const;
  [[nodiscard]] bool get_mouse_rmb_down() const;
  [[nodiscard]] bool get_mouse_mmb_down() const;

  void set_mousewheel_y(const float amount);
  [[nodiscard]] float get_mousewheel_y() const;

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
