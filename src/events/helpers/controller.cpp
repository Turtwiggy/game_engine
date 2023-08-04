// header
#include "events/helpers/controller.hpp"

#include "maths/maths.hpp"

#include <SDL2/SDL.h>
#include <iostream>

namespace game2d {

void
open_controllers(SINGLETON_InputComponent& input)
{
  int controllers = SDL_NumJoysticks();
  std::cout << "(InputManager) controllers available: " << controllers << "\n";

  for (int i = 0; i < controllers; ++i) {
    if (SDL_IsGameController(i)) {
      // Open available controllers
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller) {
        input.controllers.push_back(controller);
        std::cout << "(InputManager) controller loaded... " << i << "\n";
      } else {
        std::cerr << "Could not open gamecontroller " << i << " " << SDL_GetError() << "\n";
      }
    }
  }
};

void
process_controller_added(SINGLETON_InputComponent& input)
{
  std::cout << "controller added... processing\n";

  for (auto* controller : input.controllers) {
    if (controller)
      SDL_GameControllerClose(controller);
  };
  input.controllers.clear();

  open_controllers(input);
};

void
process_controller_removed(SINGLETON_InputComponent& input)
{
  std::cout << "controller removed... processing\n";

  for (auto* controller : input.controllers) {
    if (controller)
      SDL_GameControllerClose(controller);
  };
  input.controllers.clear();

  open_controllers(input);
};

// bool
// get_button_down(SDL_GameController* controller, SDL_GameControllerButton button)
// {
// not ideal, but the joy_event buttons dont seem
// to align with SDL_GameControllerButton enum
//   bool held = get_button_held(controller, button);

//   SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
//   SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);
//   bool has_joystick = controller_buttons_pressed.find(joystick_id) != controller_buttons_pressed.end();
//   if (!has_joystick)
//     controller_buttons_pressed.insert({ joystick_id, {} });

//   auto& buttons = controller_buttons_pressed[joystick_id];
//   auto button_location = std::find_if(
//     buttons.begin(), buttons.end(), [&button](const std::pair<uint64_t, Uint8> obj) { return obj.second == button;
//     });

//   // Button was pressed this frame, no recollection
//   if (held && button_location == buttons.end()) {
//     buttons.push_back({ this->frame, button });
//     return true;
//   }

//   // Button exists this frame
//   if (button_location != buttons.end()) {
//     // If its still the same frame, return true
//     const auto& button_added_frame = (*button_location).first;
//     if (button_added_frame == this->frame)
//       return true;
//     // If its a different frame, remove it
//     if (!held)
//       buttons.erase(button_location);
//   }
//   return false;
// }

// bool
// get_button_up(SDL_GameController* controller, SDL_GameControllerButton button)
// {
// bool pressed = get_button_down(controller, button);

//   SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
//   SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);
//   auto& buttons = controller_buttons_pressed[joystick_id];
//   auto button_location = std::find_if(
//     buttons.begin(), buttons.end(), [&button](const std::pair<uint64_t, Uint8> obj) { return obj.second == button;
//     });
//   if (!pressed && button_location != buttons.end()) {
//     return true;
//   }
//   return false;
// }

bool
get_button_held(SDL_GameController* controller, SDL_GameControllerButton button)
{
  Uint8 val = SDL_GameControllerGetButton(controller, button);
  return val != 0;
}

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