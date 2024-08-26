#include "events/helpers/fixed_update.hpp"

#include <algorithm>

namespace game2d {

bool
fixed_input_keyboard_press(const std::vector<InputEvent>& inputs, const SDL_Scancode& key)
{
  const auto l = [&key](const InputEvent& e) {
    return (e.type == InputType::keyboard && e.keyboard == key && e.state == InputState::press);
  };
  return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
};

bool
fixed_input_keyboard_held(const std::vector<InputEvent>& inputs, const SDL_Scancode& key)
{
  const auto l = [&key](const InputEvent& e) {
    return (e.type == InputType::keyboard && e.keyboard == key && e.state == InputState::held);
  };
  return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
};

bool
fixed_input_keyboard_release(const std::vector<InputEvent>& inputs, const SDL_Scancode& key)
{
  const auto l = [&key](const InputEvent& e) {
    return (e.type == InputType::keyboard && e.keyboard == key && e.state == InputState::release);
  };
  return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
};

// SDL_BUTTON_LEFT = 1; SDL_BUTTON_MIDDLE = 2; SDL_BUTTON_RIGHT = 3;
bool
fixed_input_mouse_press(const std::vector<InputEvent>& inputs, const uint32_t button)
{
  const auto& l = [&button](const InputEvent& e) {
    return (e.type == InputType::mouse && e.mouse == button && e.state == InputState::press);
  };
  return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
};

bool
fixed_input_mouse_release(const std::vector<InputEvent>& inputs, const uint32_t button)
{
  const auto& l = [&button](const InputEvent& e) {
    return (e.type == InputType::mouse && e.mouse == button && e.state == InputState::release);
  };
  return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
};

bool
fixed_input_controller_button_press(const std::vector<InputEvent>& inputs, const SDL_GameControllerButton& button)
{
  const auto& l = [&button](const InputEvent& e) {
    return (e.type == InputType::controller_button && e.state == InputState::press &&
            // e.joystick_id ==  // any id
            e.controller_button == button);
  };
  return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
};

bool
fixed_input_controller_button_held(const std::vector<InputEvent>& inputs, const SDL_GameControllerButton& button)
{
  const auto& l = [&button](const InputEvent& e) {
    return (e.type == InputType::controller_button && e.state == InputState::held && e.controller_button == button);
  };
  return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
};

float
fixed_input_controller_axis_held(const std::vector<InputEvent>& inputs, const SDL_GameControllerAxis& axis)
{
  // process more recent inputs first
  const int size = static_cast<int>(inputs.size());
  for (int i = size - 1; i >= 0; i--) {
    const auto& e = inputs[i];
    const bool is_controller_axis = e.type == InputType::controller_axis;
    const bool your_axis = e.controller_axis == axis;
    if (is_controller_axis && your_axis)
      return e.controller_axis_value_01;
  }
  return 0.0f;
};

} // namespace game2d