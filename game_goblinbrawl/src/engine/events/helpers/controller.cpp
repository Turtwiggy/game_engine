// header
#include "engine/events/helpers/controller.hpp"

#include "engine/maths/maths.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <format>

namespace game2d {

void
open_controllers(SINGLE_InputComponent& input)
{
  const int controllers = SDL_NumJoysticks();
  SDL_Log("%s", std::format("(InputManager) controllers available: {}", controllers).c_str());

  for (int i = 0; i < controllers; ++i) {
    if (SDL_IsGameController(i)) {
      // Open available controllers
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller) {
        input.controllers.push_back(controller);
        SDL_Log("%s", std::format("(InputManager) controller loaded... {}", i).c_str());
      } else {
        SDL_Log("%s", std::format("Could not open gamecontroller {}, error: {}", i, SDL_GetError()).c_str());
      }
    }
  }
};

void
process_controller_added(SINGLE_InputComponent& input)
{
  SDL_Log("%s", std::format("controller added... processing").c_str());

  for (auto* controller : input.controllers) {
    if (controller)
      SDL_GameControllerClose(controller);
  };
  input.controllers.clear();

  open_controllers(input);
};

void
process_controller_removed(SINGLE_InputComponent& input)
{
  SDL_Log("%s", std::format("controller removed... processing").c_str());

  for (auto* controller : input.controllers) {
    if (controller)
      SDL_GameControllerClose(controller);
  };
  input.controllers.clear();

  open_controllers(input);
};

void
process_button_down(SINGLE_InputComponent& input, const SDL_JoystickID& id, const SDL_GameControllerButton button)
{
  std::pair<SDL_JoystickID, SDL_GameControllerButton> evt = std::make_pair(id, button);
  input.button_down.push_back(evt);

  InputEvent e;
  e.type = InputType::controller_button;
  e.state = InputState::press;
  e.controller_button = button;
  e.joystick_id = id;
  input.unprocessed_inputs.push_back(e);
};

void
process_button_up(SINGLE_InputComponent& input, const SDL_JoystickID& id, const SDL_GameControllerButton button)
{
  std::pair<SDL_JoystickID, SDL_GameControllerButton> evt = std::make_pair(id, button);
  input.button_released.push_back(evt);

  InputEvent e;
  e.type = InputType::controller_button;
  e.state = InputState::release;
  e.controller_button = button;
  e.joystick_id = id;
  input.unprocessed_inputs.push_back(e);
};

bool
get_button_down(const SINGLE_InputComponent& input, SDL_GameController* c, const SDL_GameControllerButton& b)
{
  const auto& in = input.button_down;
  SDL_Joystick* joystick = SDL_GameControllerGetJoystick(c);
  const int32_t id = SDL_JoystickInstanceID(joystick);
  const auto& pressed =
    std::find_if(in.begin(), in.end(), [&id, &b](const std::pair<SDL_JoystickID, SDL_GameControllerButton>& e) {
      return (static_cast<int32_t>(e.first) == id && e.second == b);
    });
  return pressed != in.end();
}

bool
get_button_up(const SINGLE_InputComponent& input, SDL_GameController* c, const SDL_GameControllerButton& b)
{
  const auto& in = input.button_released;
  SDL_Joystick* joystick = SDL_GameControllerGetJoystick(c);
  const int32_t id = SDL_JoystickInstanceID(joystick);

  const auto& release =
    std::find_if(in.begin(), in.end(), [&id, &b](const std::pair<SDL_JoystickID, SDL_GameControllerButton>& e) {
      return (static_cast<int32_t>(e.first) == id && e.second == b);
    });
  return release != in.end();
}

bool
get_button_held(SDL_GameController* controller, const SDL_GameControllerButton& button)
{
  Uint8 val = SDL_GameControllerGetButton(controller, button);
  return val != 0;
};

float
get_axis_01(SDL_GameController* controller, SDL_GameControllerAxis axis)
{
  const int JOYSTICK_DEAD_ZONE = 2000;

  Sint16 val = SDL_GameControllerGetAxis(controller, axis);

  // add deadzone
  if (val < 0.0f && val > -JOYSTICK_DEAD_ZONE)
    return 0.0f;
  if (val >= 0.0f && val < JOYSTICK_DEAD_ZONE)
    return 0.0f;

  return engine::scale(val, -32768.0f, 32767.0f, -1.0f, 1.0f);
}

Sint16
get_axis_raw(SDL_GameController* controller, SDL_GameControllerAxis axis)
{
  Sint16 val = SDL_GameControllerGetAxis(controller, axis);
  return val;
}

} // namespace game2d