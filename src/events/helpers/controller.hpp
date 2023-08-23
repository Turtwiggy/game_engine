#pragma once

#include "events/components.hpp"

namespace game2d {

void
open_controllers(SINGLETON_InputComponent& input);
void
process_controller_added(SINGLETON_InputComponent& input);
void
process_controller_removed(SINGLETON_InputComponent& input);

void
process_button_down(SINGLETON_InputComponent& input, const SDL_JoystickID& id, const SDL_GameControllerButton button);
void
process_button_up(SINGLETON_InputComponent& input, const SDL_JoystickID& id, const SDL_GameControllerButton button);

// [[nodiscard]] bool
// get_button_down(SDL_GameController* controller, SDL_GameControllerButton button);
// [[nodiscard]] bool
// get_button_up(SDL_GameController* controller, SDL_GameControllerButton button);

[[nodiscard]] bool
get_button_held(SDL_GameController* controller, SDL_GameControllerButton button);
[[nodiscard]] float
get_axis_01(SDL_GameController* controller, SDL_GameControllerAxis axis);
[[nodiscard]] Sint16
get_axis_raw(SDL_GameController* controller, SDL_GameControllerAxis axis);

} // namespace game2d