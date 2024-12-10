#pragma once

#include "engine/events/components.hpp"

namespace game2d {

void
process_key_down(SINGLE_InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat);

void
process_key_up(SINGLE_InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat);

void
process_held_buttons(SINGLE_InputComponent& input);

[[nodiscard]] bool
get_key_down(const SINGLE_InputComponent& input, const SDL_Scancode button);

[[nodiscard]] bool
get_key_up(const SINGLE_InputComponent& input, const SDL_Scancode button);

[[nodiscard]] bool
get_key_held(const SINGLE_InputComponent& input, const SDL_Scancode button);

} // namespace game2d