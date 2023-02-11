#pragma once

#include "components.hpp"
#include "helpers.hpp"

#include <AL/al.h>

namespace game2d {

ALuint
get_sound_id(const SINGLETON_AudioComponent& audio, const std::string& tag);

} // namespace game2d