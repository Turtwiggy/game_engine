#pragma once

#include "events/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

#include <vector>

namespace game2d {

bool
fixed_input_keyboard_press(const std::vector<InputEvent>& inputs, const SDL_Scancode& key);

bool
fixed_input_keyboard_held(const std::vector<InputEvent>& inputs, const SDL_Scancode& key);

bool
fixed_input_mouse_press(const std::vector<InputEvent>& inputs, const int& button);

bool
fixed_input_controller_button_press(const std::vector<InputEvent>& inputs, const SDL_GameControllerButton& button);

bool
fixed_input_controller_button_held(const std::vector<InputEvent>& inputs, const SDL_GameControllerButton& button);

float
fixed_input_controller_axis_held(const std::vector<InputEvent>& inputs, const SDL_GameControllerAxis& axis);

} // namespace game2d