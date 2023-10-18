// header
#include "events/helpers/keyboard.hpp"

namespace game2d {

void
process_key_down(SINGLETON_InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat)
{
  if (is_repeat == 1)
    return; // dont process held
  input.keys_pressed.push_back(button);

  // set as held
  const auto& held = std::find(input.keys_held.begin(), input.keys_held.end(), button);
  if (held == input.keys_held.end())
    input.keys_held.push_back(button);

  InputEvent ie;
  ie.type = InputType::keyboard;
  ie.keyboard = button;
  ie.state = InputState::press;
  input.unprocessed_inputs.push_back(ie);
}

void
process_key_up(SINGLETON_InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat)
{
  if (is_repeat == 1)
    return; // dont process held
  input.keys_released.push_back(button);

  // remove from held
  const auto& held = std::find(input.keys_held.begin(), input.keys_held.end(), button);
  if (held != input.keys_held.end())
    input.keys_held.erase(held);

  InputEvent ie;
  ie.type = InputType::keyboard;
  ie.keyboard = button;
  ie.state = InputState::release;
  input.unprocessed_inputs.push_back(ie);
}

// iterate over held buttons and resubmit as held
void
process_held_buttons(SINGLETON_InputComponent& input)
{
  for (const auto& held : input.keys_held) {
    InputEvent ie;
    ie.type = InputType::keyboard;
    ie.keyboard = held;
    ie.state = InputState::held;
    input.unprocessed_inputs.push_back(ie);
  }
}

bool
get_key_down(const SINGLETON_InputComponent& input, const SDL_Scancode button)
{
  const auto& press = std::find(input.keys_pressed.begin(), input.keys_pressed.end(), button);
  return press != input.keys_pressed.end();
}

bool
get_key_up(const SINGLETON_InputComponent& input, const SDL_Scancode button)
{
  const auto& release = std::find(input.keys_released.begin(), input.keys_released.end(), button);
  return release != input.keys_released.end();
}

bool
get_key_held(const SINGLETON_InputComponent& input, const SDL_Scancode button)
{
  const auto& held = std::find(input.keys_held.begin(), input.keys_held.end(), button);
  return held != input.keys_held.end();
  // return input.state[button];
}

} // namespace game2d