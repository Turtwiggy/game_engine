// header
#include "modules/events/helpers/keyboard.hpp"

namespace game2d {

void
process_key_down(InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat)
{
  if (is_repeat == 1)
    return; // dont process held
  input.keys_pressed.push_back(button);
}

void
process_key_up(InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat)
{
  if (is_repeat == 1)
    return; // dont process held
  input.keys_released.push_back(button);
}

bool
get_key_down(const InputComponent& input, const SDL_Scancode button)
{
  const auto& button_pressed = std::find(input.keys_pressed.begin(), input.keys_pressed.end(), button);
  return button_pressed != input.keys_pressed.end();
}

bool
get_key_up(const InputComponent& input, const SDL_Scancode button)
{
  const auto& button_pressed = std::find(input.keys_released.begin(), input.keys_released.end(), button);
  return button_pressed != input.keys_released.end();
}

bool
get_key_held(const InputComponent& input, const SDL_Scancode button)
{
  return input.state[button];
}

} // namespace game2d