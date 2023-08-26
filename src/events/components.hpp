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
  controller,
};

enum class InputState
{
  press,
  held,
  release,
};

enum class JoystickEventType
{
  axis,
  button,
  hat,
};

struct InputEvent
{
  InputType type;
  InputState state;

  uint32_t mouse = 0;

  SDL_Scancode keyboard;

  JoystickEventType joystick_event;
  SDL_JoystickID joystick_id = 0;

  // axis
  Uint8 controller_axis = 0;
  float controller_axis_value_01 = 0;
  // button
  Uint8 controller_button = 0;
  // hat
  Uint8 controller_hat = 0;
  Uint8 controller_hat_value = 0;
};

struct SINGLETON_FixedUpdateInputHistory
{
  // techniques to combat packet loss & high latency:
  // - sliding server buffer to contain more history of inputs
  // - contract fixed time to e.g. 16 -> 15.2 to send inputs more frequently on client

  // send size of history from last ack frame to current frame
  int fixed_tick_since_ack = 0;
  int fixed_tick = 0;

  std::map<uint32_t, std::vector<InputEvent>> history;

  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLETON_FixedUpdateInputHistory, fixed_tick_since_ack, history);
};

struct SINGLETON_InputComponent
{
  // keys pressed in Update() to be processed in FixedUpdate()
  // note: this is deliberately not a Set incase a user e.g.
  // press W, release W, press W before one FixedUpdate() occurs.
  std::vector<InputEvent> unprocessed_inputs;

  // keyboard
  const Uint8* state;
  std::vector<SDL_Scancode> keys_pressed;
  std::vector<SDL_Scancode> keys_released;

  // connected controllers
  std::vector<SDL_GameController*> controllers;
  std::vector<std::pair<SDL_JoystickID, SDL_GameControllerButton>> button_down;
  std::vector<std::pair<SDL_JoystickID, SDL_GameControllerButton>> button_released;

  bool update_since_last_fixed_update = false;
};

} // namespace game2d