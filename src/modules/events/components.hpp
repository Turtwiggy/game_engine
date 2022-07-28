#pragma once

#include <glm/glm.hpp>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

#include <map>
#include <queue>
#include <vector>

namespace game2d {

struct AppEvent
{
  SDL_Event evt;
  int frame = 0;
};

struct SINGLETON_InputComponent
{
  std::queue<AppEvent> sdl_events;

  // mouse
  const int CURSOR_SIZE = 16;
  glm::ivec2 mouse_position_in_worldspace = glm::ivec2(0);
  glm::ivec2 mouse_drag_boundingbox = glm::ivec2(0);
  glm::ivec2 mouse_click;

  // keyboard
  const Uint8* state;
  std::vector<SDL_Scancode> keys_pressed;
  std::vector<SDL_Scancode> keys_released;

  // connected controllers
  // std::vector<SDL_GameController*> controllers;

  // joystick-id, to frame-button associaton
  // std::map<SDL_JoystickID, std::vector<std::pair<uint64_t, Uint8>>> controller_buttons_pressed;
};

// struct PlayerKeyboardComponent
// {
//   SDL_Scancode kb_start = SDL_SCANCODE_RETURN;
//   SDL_Scancode kb_up = SDL_SCANCODE_W;
//   SDL_Scancode kb_left = SDL_SCANCODE_A;
//   SDL_Scancode kb_down = SDL_SCANCODE_S;
//   SDL_Scancode kb_right = SDL_SCANCODE_D;
//   SDL_Scancode kb_space = SDL_SCANCODE_SPACE;
// };

// struct PlayerControllerComponent
// {
//   // controller bindings
//   SDL_GameControllerAxis c_left_stick_x = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX;
//   SDL_GameControllerAxis c_left_stick_y = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY;
//   SDL_GameControllerAxis c_right_stick_x = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX;
//   SDL_GameControllerAxis c_right_stick_y = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY;
//   SDL_GameControllerAxis c_left_trigger = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT;
//   SDL_GameControllerAxis c_right_trigger = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
//   SDL_GameControllerButton c_start = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START;
//   SDL_GameControllerButton c_a = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A;
//   SDL_GameControllerButton c_b = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B;
//   SDL_GameControllerButton c_x = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X;
//   SDL_GameControllerButton c_y = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y;
//   SDL_GameControllerButton c_dpad_up = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP;
//   SDL_GameControllerButton c_dpad_down = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN;
//   SDL_GameControllerButton c_dpad_left = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT;
//   SDL_GameControllerButton c_dpad_right = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
// };

} // namespace game2d