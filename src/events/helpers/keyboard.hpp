#pragma once

#include "events/components.hpp"

// other library
#include "SDL2/SDL_keycode.h"

namespace game2d {

void
process_key_down(SINGLETON_InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat);

void
process_key_up(SINGLETON_InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat);

void
process_held_buttons(SINGLETON_InputComponent& input);

[[nodiscard]] bool
get_key_down(const SINGLETON_InputComponent& input, const SDL_Scancode button);

[[nodiscard]] bool
get_key_up(const SINGLETON_InputComponent& input, const SDL_Scancode button);

[[nodiscard]] bool
get_key_held(const SINGLETON_InputComponent& input, const SDL_Scancode button);

} // namespace game2d