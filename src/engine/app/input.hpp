#pragma once

// other library
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

// c++ standard lib headers
#include <map>
#include <utility>
#include <vector>

namespace engine {

// This class gets keyboard input, and gets controller input
class InputManager
{
public:
  InputManager();
  void new_frame(const uint64_t& frame);
  void process_key_down(const SDL_Scancode button, const Uint8 is_repeat);
  void process_key_up(const SDL_Scancode button, const Uint8 is_repeat);
  void process_controller_added();
  void process_controller_removed();

  // keyboard
  [[nodiscard]] bool get_key_down(SDL_Scancode button);
  [[nodiscard]] bool get_key_up(SDL_Scancode button);
  [[nodiscard]] bool get_key_held(SDL_Scancode button) const;

  // mouse
  // note: _down will occur on the same frame as first _held
  // note: _release will occur on the frame after last _held
  [[nodiscard]] bool get_mouse_lmb_held() const;
  [[nodiscard]] bool get_mouse_rmb_held() const;
  [[nodiscard]] bool get_mouse_mmb_held() const;
  [[nodiscard]] bool get_mouse_lmb_press() const;
  [[nodiscard]] bool get_mouse_rmb_press() const;
  [[nodiscard]] bool get_mouse_mmb_press() const;
  [[nodiscard]] bool get_mouse_lmb_release();
  [[nodiscard]] bool get_mouse_rmb_release();
  [[nodiscard]] bool get_mouse_mmb_release();
  [[nodiscard]] glm::ivec2 get_mouse_pos() const;
  void set_mousewheel_y(const float amount);
  [[nodiscard]] float get_mousewheel_y() const;

  // controller
  void open_controllers();
  [[nodiscard]] bool get_button_down(SDL_GameController* controller, SDL_GameControllerButton button);
  [[nodiscard]] bool get_button_up(SDL_GameController* controller, SDL_GameControllerButton button);
  [[nodiscard]] bool get_button_held(SDL_GameController* controller, SDL_GameControllerButton button);
  [[nodiscard]] float get_axis_dir(SDL_GameController* controller, SDL_GameControllerAxis axis);
  // [[nodiscard]] Sint16 get_axis_raw(SDL_GameController* controller, SDL_GameControllerAxis axis);

private:
  // keyboard state
  const Uint8* state;
  std::vector<SDL_Scancode> keys_pressed;
  std::vector<SDL_Scancode> keys_released;

  // mouse state
  float mousewheel_y;

  // controller state
  uint64_t frame = 0;
  // joystick-id, to frame-button associaton
  std::map<SDL_JoystickID, std::vector<std::pair<uint64_t, Uint8>>> controller_buttons_pressed;

public:
  // temp
  std::vector<SDL_GameController*> controllers; // connected controllers
};

} // namespace engine
