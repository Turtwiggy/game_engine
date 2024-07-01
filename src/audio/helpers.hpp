#pragma once

#include "components.hpp"

#include <string>

namespace game2d {

Sound
get_sound(const SINGLETON_AudioComponent& audio, const std::string& tag);

} // namespace game2d