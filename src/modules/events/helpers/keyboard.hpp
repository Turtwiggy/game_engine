#pragma once

#include "modules/events/components.hpp"

// other library
#include <SDL2/SDL_keycode.h>

namespace game2d {

void
process_key_down(InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat);

void
process_key_up(InputComponent& input, const SDL_Scancode button, const Uint8 is_repeat);

[[nodiscard]] bool
get_key_down(const InputComponent& input, const SDL_Scancode button);

[[nodiscard]] bool
get_key_up(const InputComponent& input, const SDL_Scancode button);

[[nodiscard]] bool
get_key_held(const InputComponent& input, const SDL_Scancode button);

} // namespace game2d