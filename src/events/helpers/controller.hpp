#pragma once

#include "events/components.hpp"

namespace game2d {

void
open_controllers(SINGLETON_InputComponent& input);

void
process_controller_added(SINGLETON_InputComponent& input);
void
process_controller_removed(SINGLETON_InputComponent& input);

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