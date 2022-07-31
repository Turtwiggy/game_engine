#pragma once

#include <glm/glm.hpp>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <map>
#include <queue>
#include <vector>

namespace game2d {

struct InputEvent
{
  entt::entity player;
  SDL_Scancode key;
  bool release = false;
  bool mouse = false;

  bool controller = false;
  SDL_GameControllerButton button;
  SDL_GameControllerAxis axis;
};

struct SINGLETON_FixedUpdateInputHistory
{
  // techniques to combat packet loss & high latency
  // sliding server buffer to contain more history of inputs
  // contract fixed time to e.g. 16 -> 15.2 to send inputs more frequently on client
  // send size of history from last ack frame to current frame
  int max_history_size = 15; // could be the time between an ack frame vs cur frame
  std::queue<std::vector<InputEvent>> history;

  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLETON_FixedUpdateInputHistory, max_history_size, history)
};

struct SINGLETON_InputComponent
{
  std::vector<SDL_Event> sdl_events;
  std::vector<InputEvent> unprocessed_update_inputs;

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
  // joystick-id, to button associaton
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