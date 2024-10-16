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
#include <vector>

namespace game2d {

enum class InputType
{
  mouse,
  keyboard,
  controller_axis,
  controller_button,
  controller_hat,
};

enum class InputState
{
  press,
  held,
  release,
};

struct InputEvent
{
  InputType type;
  InputState state;

  uint32_t mouse = 0;
  SDL_Scancode keyboard;

  SDL_JoystickID joystick_id = 0;
  SDL_GameControllerButton controller_button = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_INVALID;
  SDL_GameControllerAxis controller_axis = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_INVALID;
  float controller_axis_value_01 = 0;

  // hat: not currentely implemented
  // Uint8 controller_hat = 0;
  // Uint8 controller_hat_value = 0;
};

struct SINGLE_FixedUpdateInputHistory
{
  // techniques to combat packet loss & high latency:
  // - sliding server buffer to contain more history of inputs
  // - contract fixed time to e.g. 16 -> 15.2 to send inputs more frequently on client

  // send size of history from last ack frame to current frame
  int fixed_tick_since_ack = 0;
  int fixed_tick = 0;

  std::map<uint32_t, std::vector<InputEvent>> history;

  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_FixedUpdateInputHistory, fixed_tick_since_ack, history);
};

struct SINGLE_InputComponent
{
  // keys pressed in Update() to be processed in FixedUpdate()
  // note: this is deliberately not a Set incase a user e.g.
  // press W, release W, press W before one FixedUpdate() occurs.
  std::vector<InputEvent> unprocessed_inputs;

  // keyboard
  const Uint8* state;
  std::vector<SDL_Scancode> keys_pressed;
  std::vector<SDL_Scancode> keys_released;
  std::vector<SDL_Scancode> keys_held;

  // connected controllers
  std::vector<SDL_GameController*> controllers;
  std::vector<std::pair<SDL_JoystickID, SDL_GameControllerButton>> button_down;
  std::vector<std::pair<SDL_JoystickID, SDL_GameControllerButton>> button_released;

  bool update_since_last_fixed_update = false;
};

} // namespace game2d